
package Foomatic::PPD;

use Foomatic::UIElem;

my @Sections = qw(JCLBegin 
		  JCLSetup
		  JCLToPSInterpreter
		  ExitServer
		  Prolog
		  DocumentSetup
		  PageSetup);

sub new {
    my ($type, $filename, $poid) = @_;

    open PPD, "$filename" or die;

    my %ppd;

    $ppd{'filename'} = $filename;
    my ($opt, $optname);
    my ($choice,$choiceverb,$snippet);
    my $l;
    for $l (<PPD>) {
	
	# skip comments
	next if ($l =~ m!^*%!);
	
	# skip blank lines
	next if ($l =~ m!^\s*$!);
	
	if (defined($snippet)) {
	    $snippet = "$snippet$l";
	    if ($snippet =~ s!\"\s*$!!) {
		$opt->add_option($choice, $choiceverb, $snippet);
		$snippet = undef;
	    }
	} elsif (defined($opt)) {      
	    # In mid-parse of a UI option clause
	    if ($l =~ m!^\*CloseUI: \*$optname!) {
		# close up the $opt object
		push(@{$ppd{'options'}}, $opt);
		$optname = $opt = undef;
	    } elsif ($l =~ m!^\*OrderDependency: ([\.\d]+) (AnySetup|JCLSetup|PageSetup|DocumentSetup|Prolog|ExitServer) \*$optname!) {
		my ($order, $section) = ($1, $2);
		$opt->set('order_real'=>$order);
		$opt->set('order_section'=>$section);
		$opt->set('order_keyword'=>$optname);
	    } elsif ($l =~ m!^\*Default$optname:\s+([0-9\&A-Za-z]+)!) {
		$opt->set('default'=>$1);
	    } elsif ($l =~ m!^\*$optname ([\d\&A-Za-z]+)(\/([^:]+))?:\s*\"([^\"]*)(\")?!) {
		if ($5 eq '"') {
		    $opt->add_option($1,$3?$3:$1,$4);
		} else {
		    $snippet = $4;
		    $choice = $1;
		    $choiceverb = $3?$3:$1;
		}
	    }
	    
	} else {
	    # not in UI clause parsing
	    if ($l =~ m!^\*NickName:\s*\"(.+)\"!) {
		$ppd{'NickName'} = $1;
	    } elsif ($l =~ m!^\*ModelName:\s*\"(.+)\"!) {
		$ppd{'ModelName'} = $1;
	    } elsif ($l =~ m!^\*Manufacturer:\s*\"(.+)\"!) {
		$ppd{'Manufacturer'} = $1;
	    } elsif ($l =~ m!^\*OpenUI \*([0-9\&A-Za-z]+)(\/(.+))?:\s*(Boolean|PickOne|PickMany)!) {
		my ($name, $label, $type) = ($1, $3, $4);
		# make new $opt object
		$optname = $name;
		$opt = new Foomatic::UIElem ('name'=>$optname,
					     'type'=>$type,
					     'label'=>($label?$label:$name));
	    } 
	    
	    # yadda...
	    
	} 

    }
    
    close PPD;

    my $this = bless \%ppd;
    $this->sort_options();
    $this->{'printer_id'} = $poid;

    return $this;
}

sub sort_options {
    my $this = $_[0];

    # First, sort all the options into sections
    my %sections;
    my $o;
    while (defined($o=pop(@{$this->{'options'}}))) {
	my $sec = $o->{'order_section'};
	push(@{$sections{($sec ? $sec : 'Queries')}}, $o);
    }
    $this->{'options'} = undef;

    # Put AnySetup stuff in DocumentSetup (or could be PageSetup)
    push(@{$sections{'DocumentSetup'}}, @{$sections{'AnySetup'}});
    @{$sections{'AnySetup'}} = ();

    # Now sort each section by orderdep number
    my $k;
    for $k (keys(%sections)) {
	my @sorted = sort {$a->{'order_real'} 
			   <=> $b->{'order_real'}}   @{$sections{$k}};
	$sections{$k} = \@sorted;
    }

    $this->{'sections'} = \%sections;
}


sub pdq_options {
    my $this = $_[0];

    my @opts;

    my $k;
    for $k (@Sections) {
	my $o;
	for $o (@{$this->{'sections'}->{$k}}) {
	    my ($name, $label, $type, $default) = 
		($o->{'name'},$o->{'label'},$o->{'type'},$o->{'default'});

	    if ($type eq 'PickOne' 
		or $type eq 'Boolean'
		or $type eq 'PickMany') {
		push(@opts,
		     "  option {\n",
		     "    var = \"$name\"\n",
		     "    desc = \"$label\"\n",
		     "    default_choice \"$default\"\n");

		my $c;
		for $c (@{$o->{'options'}}) {
		    my ($label, $option) = ($c->{'label'}, $c->{'option'});
		    push(@opts,
			 "    choice \"$name:$option\" {\n",
			 "      value = \"$option\"\n",
			 "      desc = \"$label\"\n",
			 "    }\n");
		}

		push(@opts,
		     "  }\n");
	    }

	    $optnum++;
	}
    }

    return @opts;
}

sub _tag {
    my ($t, @v) = @_;

    return '' if !defined(@v);

    if (0) {
	$v =~ s!\&!\&amp\;!g;
	$v =~ s!\<!\&lt\;!g;
    }

    return "<$t>" . join('',@v) . "</$t>\n";
}

sub foo_options {
    my ($this) = (@_);

    # We build a list of option xml objects
    my @options;

    # All of them need to get sandwiched between
    # %!PS
    # statusdict begin
    #   opt1
    #   opt2
    #   ....
    # end

    # For each section in the order defined by Adobe, and for each
    # option in the order defined by the PPD, we emit shell code to
    # generate the proper snippet-o-postscript into the backend
    # filter's output.

    # TODO: some sections go in the middle of the document!  E-gad!
    # TODO: JCL incantations - these aren't even parsed from the PPD yet.

    my $prn = $this->{'printer_id'};
    my $filename = $this->{'filename'};
    
    my $k;
    # AnySetup has already been moved to DocumentSetup.
    # We don't do the JCL stuff yet.
    for $k (qw(Prolog DocumentSetup PageSetup)) {
	my $o;
	my $optidx = 0;
	for $o (@{$this->{'sections'}->{$k}}) {

	    my (@opt) = ();

	    # We only do PickOne and Boolean (no PickMany)
	    if ($o->{'type'} ne 'PickOne'
		and $o->{'type'} ne 'Boolean') {
		print STDERR ("Skipping option ", $o->{'name'}, 
			      " because it is a ", $o->{'type'}, "\n");
		next;
	    }

	    # Skip "PageRegion", it is the same as "PageSize"
	    next if ($o->{'name'} eq "PageRegion");

	    my ($var,$order) = @$o{'name','order_real'};

	    # Find index of default option
	    my $c;
	    my $scanindex = 1;
	    my $defaultindex = 1; # assume first
	    for $c (@{$o->{'options'}}) {
		my $v = $c->{'option'};
		
		if ($o->{'default'} eq $v) {
		    $defaultindex = $scanindex;
		    last;
		}
		
		$scanindex++;
	    }


	    push (@opt, ("<option type='enum' id='ppd-$prn-$var'>\n",
			 "<!-- option from section $k in $filename -->\n",
			 _tag('arg_longname',
			      _tag('en', xml_esc($o->{'label'}))),
			 _tag('arg_shortname',
			      _tag('en', xml_esc($var))),
			 _tag('arg_execution',
			      _tag('arg_order', 500 + $order),
			      _tag('arg_spot', 'A'),
			      "<arg_postscript section='$k' />\n",
			      _tag('arg_proto', '%s')),
			 _tag('constraints',
			      ("<constraint sense='true'>\n",
			       _tag('driver', 'ppd'),
			       _tag('printer', "printer/$prn"),
			       _tag('arg_defval', "ppd-$prn-$var-$defaultindex"),
			       "</constraint>\n"))));
	    
	    my $choiceidx=0;
	    my @evals;
	    for $c (@{$o->{'options'}}) {
		my $v = $c->{'option'};
		my $snippet = $c->{'snippet'};
		$choiceidx++;
		
		if (defined($snippet)) {
		    
		    push (@evals,
			  ("<enum_val id='ppd-$prn-$var-$choiceidx'>\n",
			   _tag('ev_longname', 
				_tag('en', xml_esc($c->{'label'}))),
			   _tag('ev_shortname',
				_tag('en', xml_esc($c->{'option'}))),
			   ($snippet ? _tag('ev_driverval', xml_esc($snippet))
			    : "<ev_driverval></ev_driverval>\n"),
			   "</enum_val>\n"));
		    
		    # TODO: We should also handle <##> hex numbers in snippets!
		}
	    }

	    # If there are choices, put them in
	    if (scalar(@evals)) {
		push(@opt,
		     _tag('enum_vals', join('',@evals)));
	    
		push (@opt, "</option>\n");

		# Note that we skip the whole thing if there are no choices!
		push (@options, { 'id' => "ppd-$prn-$var",
				  'xml' => \@opt } );
	    }
	}
    }
    
    return @options;

}	

sub xml_esc {
    my ($in) = (@_);
    
    $in =~ s!&!&amp;!g;
    $in =~ s!<!&lt;!g;
    $in =~ s!>!&gt;!g;

    return $in;
}

sub pdq_filter {
    my ($this) = $_[0];

    my @filt;

    push(@filt,
	 "  filter_exec {\n\n",
	 "    echo '%!PS' > \$OUTPUT\n",
	 "    echo 'statusdict begin' >> \$OUTPUT\n\n");


    # For each section in the order defined by Adobe, and for each
    # option in the order defined by the PPD, we emit shell code to
    # generate the proper snippet-o-postscript into the PDQ filter's
    # output.
    #
    # TODO: some sections go in the middle of the document!  E-gad!
    # TODO: JCL incantations - these aren't even parsed from the PPD yet.

    my $k;
    for $k (@Sections) {
	my $o;
	for $o (@{$this->{'sections'}->{$k}}) {
	    my ($var,$order) = @$o{'name','order_real'};
	    push (@filt,
		  "    # We put option $var in section $k order numer $order\n");
	    my $c;
	    my $first = 1;
	    for $c (@{$o->{'options'}}) {
		my $el = ($first ? '' : 'el');
		my $v = $c->{'option'};
		my $snippet = $c->{'snippet'};

		if ($snippet) {
		    my @sniplines = split("\n", $snippet);
		    push(@filt, 
			 "    ${el}if [ \"\$$var\" = \"$v\" ]; then\n");
		    for (@sniplines) {
			next if /^\s*$/;
			$_ =~ s!\'!\\\'!g; # escape single quotes
			# TODO: We should also handle <##> hex numbers!
			push(@filt,
			     "      echo \'$_\' >> \$OUTPUT;\n");
		    }

		    $first = 0;
		}
	    }
	    push (@filt, "    fi\n\n");
	}
    }

    push (@filt,
	  "    echo 'end' >> \$OUTPUT\n\n",
	  "    cat \$INPUT >> \$OUTPUT\n",
	  "  }\n");

    return @filt;
}

sub pdq_driver {
    my ($this) = @_;

    my $name = $this->get_name();
    my $driver = $name;

    $driver =~ s!^\s*!!;
    $driver =~ s!\s*$!!;
    $driver =~ s!(\s+)!\-!g;
    $driver = "$driver-0.1";

    my @drv;
    push (@drv,
	  "driver $driver {\n\n",
	  "  help \"This driver was automagically converted from the \n",
	  "        PPD file for the $name by ppdtopdq.\"\n\n",
	  $this->pdq_options(),
	  "\n",
	  "  language_driver ps {\n",
	  "    filetype_regx = \"postscript\"\n",
	  "  }\n\n",
	  $this->pdq_filter(),
	  "}\n");

    return @drv;
}

# get a nice pretty english name for this thing
sub get_name {
    my ($this) = @_;

    my ($mk,$md,$nk) = (@$this{'Manufacturer','ModelName','NickName'});

    my $name;
    if ($mk) { $name = "$mk"; }
    if ($md) { $name = "$name $md"; }
    elsif ($nk) { $name = "$name $nk"; }

    return $name;
}

1;
