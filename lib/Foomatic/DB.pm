
package Foomatic::DB;
use Exporter;
@ISA = qw(Exporter);

@EXPORT_OK = qw(normalizename comment_filter
		get_overview
		getexecdocs
		);
@EXPORT = qw(ppdtoperl);

use Foomatic::Defaults qw(:DEFAULT $DEBUG);
use Data::Dumper;
use POSIX;                      # for rounding integers

my $ver = '$Revision$ ';

# constructor for Foomatic::DB
sub new {
    my $type = shift(@_);
    my $this = bless {@_}, $type;
    return $this;
}

# A map from the database's internal one-letter types to English
my %driver_types = ('F' => 'Filter',
		    'P' => 'Postscript',
		    'U' => 'Ghostscript Uniprint',
		    'G' => 'Ghostscript');

# List of driver names
sub get_driverlist {
    my ($this) = @_;
    return $this->_get_xml_filelist('source/driver');
}

# List of printer id's
sub get_printerlist {
    my ($this) = @_;
    return $this->_get_xml_filelist('source/printer');
}

sub get_overview {
    my ($this, $rebuild) = @_;

    # "$this->{'overview'}" is a memory cache only for the current process
    if ((!defined($this->{'overview'}))
	or (defined($rebuild) and $rebuild)) {
	# Generate overview Perl data structure from database
	my $VAR1;
	eval (`$bindir/foomatic-combo-xml -O -l '$libdir' | $bindir/foomatic-perl-data -O`) ||
	    die ("Could not run \"foomatic-combo-xml\"/\"foomatic-perl-data\"!");
	$this->{'overview'} = $VAR1;
    }

    return $this->{'overview'};
}

sub get_overview_xml {
    my ($this, $compile) = @_;

    open( FCX, "$bindir/foomatic-combo-xml -O -l '$libdir'|")
	or die "Can't execute $bindir/foomatic-combo-xml -O -l '$libdir'";
    $_ = join('', <FCX>);
    close FCX;
    return $_;
}

sub get_combo_data_xml {
    my ($this, $drv, $poid, $withoptions) = @_;

    # Insert the default option settings if there are some and the user
    # desires it.
    my $options = "";
    if (($withoptions) && (defined($this->{'dat'}))) {
	my $dat = $this->{'dat'};
	for $arg (@{$dat->{'args'}}) {
	    my $name = $arg->{'name'};
	    my $default = $arg->{'default'};
	    if (($name) && ($default)) {
		$options .= " -o '$name'='$default'";
	    }
	}
    }

    open( FCX, "$bindir/foomatic-combo-xml -d '$drv' -p '$poid'$options -l '$libdir'|")
	or die "Can't execute $bindir/foomatic-combo-xml -d '$drv' -p '$poid'$options -l '$libdir'";
    $_ = join('', <FCX>);
    close FCX;
    return $_;
}

sub get_printer {
    my ($this, $poid) = @_;
    # Generate printer Perl data structure from database
    my $VAR1;
    if (-r "$libdir/db/source/printer/$poid.xml") {
	eval (`$bindir/foomatic-perl-data -P '$libdir/db/source/printer/$poid.xml'`) ||
	    die ("Could not run \"foomatic-perl-data\"!");
    } else {
	return undef;
    }
    return $VAR1;
}

sub get_printer_xml {
    my ($this, $poid) = @_;
    return $this->_get_object_xml("source/printer/$poid", 1);
}

sub get_driver {
    my ($this, $drv) = @_;
    # Generate driver Perl data structure from database
    my $VAR1;
    if (-r "$libdir/db/source/driver/$drv.xml") {
	eval (`$bindir/foomatic-perl-data -D '$libdir/db/source/driver/$drv.xml'`) ||
	    die ("Could not run \"foomatic-perl-data\"!");
    } else {
	return undef;
    }
    return $VAR1;
}

sub get_driver_xml {
    my ($this, $drv) = @_;
    return $this->_get_object_xml("source/driver/$drv", 1);
}

# Utility query function sorts of things:

sub get_printers_for_driver {
    my ($this, $drv) = @_;

    my $driver = $this->get_driver($drv);

    if (!defined($driver)) {return undef;}

    return map { $_->{'id'} } @{$driver->{'printers'}};
}

# Routine lookup; just examine the overview
sub get_drivers_for_printer {
    my ($this, $printer) = @_;

    my @drivers = ();

    my $over = $this->get_overview();

    my $p;
    for $p (@{$over}) {
	if ($p->{'id'} eq $printer) {
	    return @{$p->{'drivers'}};
	}
    }

    return undef;
}

# This function sorts the options at first by their group membership and
# then by their names appearing in the list of functional areas. This way
# it will be made easier to build the PPD file with option groups and in
# user interfaces options will appear sorted by their functionality.
sub sortargs {

    # All sorting done case-insensitive and characters which are not a
    # letter or number are taken out!!

    # List of typical option names to appear at first
    # The terms must fit to the beginning of the line, terms which must fit
    # exactly must have '\$' in the end.
    my @standardopts = (
			# The most important composite option
			"printoutmode",
			# Options which appear in the "General" group in 
			# CUPS and similar media handling options
			"pagesize",
			"papersize",
			"mediasize",
			"inputslot",
			"papersource",
			"mediasource",
			"sheetfeeder",
			"mediafeed",
			"paperfeed",
			"manualfeed",
			"manual",
			"outputtray",
			"outputslot",
			"outtray",
			"faceup",
			"facedown",
			"mediatype",
			"papertype",
			"mediaweight",
			"paperweight",
			"duplex",
			"sides",
			"binding",
			"tumble",
			"notumble",
			"media",
			"paper",
			# Other hardware options
			"inktype",
			"ink",
			# Page choice/ordering options
			"pageset",
			"pagerange",
			"pages",
			"nup",
			"numberup",
			# Printout quality, colour/bw
			"resolution",
			"gsresolution",
			"hwresolution",
			"quality",
			"printquality",
			"printoutquality",
			"bitsperpixel",
			"photomode",
			"photo",
			"colormode",
			"colourmode",
			"color",
			"colour",
			"grayscale",
			"gray",
			"monochrome",
			"mono",
			"blackonly",
			"colormodel",
			"colourmodel",
			"processcolormodel",
			"processcolourmodel",
			"printcolors",
			"printcolours",
			"outputtype",
			"outputmode",
			"printingmode",
			"printoutmode",
			"printmode",
			"mode",
			"imagetype",
			"imagemode",
			"image",
			"dithering",
			"dither",
			"halftoning",
			"halftone",
			"floydsteinberg",
			"ret\$",
			"cret\$",
			"photoret\$",
			# Adjustments
			"gammacorrection",
			"gammacorr",
			"gammageneral",
			"mastergamma",
			"stpgamma",
			"gammablack",
			"gammacyan",
			"gammamagenta",
			"gammayellow",
			"gamma",
			"density",
			"stpdensity",
			"hpljdensity",
			"tonerdensity",
			"inkdensity",
			"brightness",
			"stpbrightness",
			"saturation",
			"stpsaturation",
			"hue",
			"stphue",
			"tint",
			"stptint",
			"contrast",
			"stpcontrast",
			"black",
			"stpblack",
			"cyan",
			"stpcyan",
			"magenta",
			"stpmagenta",
			"yellow",
			"stpyellow"
			);

    my @standardgroups = (
			  "general",
			  "media",
			  "quality",
			  "imag",
			  "color",
			  "output",
			  "finish",
			  "stapl",
			  "extra",
			  "install"
			  );

    my $compare;

    # Argument records
    my $firstarg = $a;
    my $secondarg = $b;

    # Bring the two option names into a standard form to compare them
    # in a better way
    my $first = normalizename(lc($firstarg->{'name'}));
    $first =~ s/[\W_]//g;
    my $second = normalizename(lc($secondarg->{'name'}));
    $second =~ s/[\W_]//g;

    # group names
    my $firstgr = $firstarg->{'group'};
    my @firstgroup = split("/", $firstgr); 
    my $secondgr = $secondarg->{'group'};
    my @secondgroup = split("/", $secondgr);

    my $i = 0;

    # Compare groups
    while ($firstgroup[$i] && $secondgroup[$i]) {

	# Normalize group names
	my $firstgr = normalizename(lc($firstgroup[$i]));
	$firstgr =~ s/[\W_]//g;
	my $secondgr = normalizename(lc($secondgroup[$i]));
	$secondgr =~ s/[\W_]//g;
	    
	# Are the groups in the list of standard group names?
	my $j;
	for ($j = 0; $j <= $#standardgroups; $j++) {
	    my $firstinlist = ($firstgr =~ /^$standardgroups[$j]/);
	    my $secondinlist = ($secondgr =~ /^$standardgroups[$j]/);
	    if (($firstinlist) && (!$secondinlist)) {return -1};
	    if (($secondinlist) && (!$firstinlist)) {return 1};
	    if (($firstinlist) && ($secondinlist)) {last};
	}

	# Compare normalized group names
	$compare = $firstgr cmp $secondgr;
	if ($compare != 0) {return $compare};

	# Compare original group names
	$compare = $firstgroup[$i] cmp $secondgroup[$i];
	if ($compare != 0) {return $compare};
	
	$i++;
    }

    # The one with a deeper level in the group tree will come later
    if ($firstgroup[$i]) {return 1};
    if ($secondgroup[$i]) {return -1};

    # Check whether they argument names are in the @standardopts list
    for ($i = 0; $i <= $#standardopts; $i++) {
	my $firstinlist = ($first =~ /^$standardopts[$i]/);
	my $secondinlist = ($second =~ /^$standardopts[$i]/);
	if (($firstinlist) && (!$secondinlist)) {return -1};
	if (($secondinlist) && (!$firstinlist)) {return 1};
	if (($firstinlist) && ($secondinlist)) {last};
    }

    # None of the search terms in the list, compare the standard-formed
    # strings
    $compare = ( $first cmp $second );
    if ($compare != 0) {return $compare};

    # No other criteria fullfilled, compare the original input strings
    return $firstarg->{'name'} cmp $secondarg->{'name'};
}

sub sortvals {

    # All sorting done case-insensitive and characters which are not a letter
    # or number are taken out!!

    # List of typical choice names to appear at first
    # The terms must fit to the beginning of the line, terms which must fit
    # exactly must have '\$' in the end.
    my @standardvals = (
			# Default setting
			"default",
			"printerdefault",
			# Paper sizes
			"letter\$",
			#"legal",
			"a4\$",
			# Paper types
			"plain",
			# Printout Modes
			"draft\$",
			"draft\.gray",
			"draft\.mono",
			"draft\.",
			"draft",
			"normal\$",
			"normal\.gray",
			"normal\.mono",
			"normal\.",
			"normal",
			"high\$",
			"high\.gray",
			"high\.mono",
			"high\.",
			"high",
			"veryhigh\$",
			"veryhigh\.gray",
			"veryhigh\.mono",
			"veryhigh\.",
			"veryhigh",
			"photo\$",
			"photo\.gray",
			"photo\.mono",
			"photo\.",
			"photo",
			# Trays
			"upper",
			"top",
			"middle",
			"mid",
			"lower",
			"bottom",
			"highcapacity",
			"multipurpose",
			"tray",
			);

    # Do not waste time if the input strings are equal
    if ($a eq $b) {return 0;}

    # Are the two strings numbers? Compare them numerically
    if (($a =~ /^[\d\.]+$/) && ($b =~ /^[\d\.]+$/)) {
	my $compare = ( $a <=> $b );
	if ($compare != 0) {return $compare};
    }

    # Bring the two option names into a standard form to compare them
    # in a better way
    my $first = lc($a);
    $first =~ s/[\W_]//g;
    my $second = lc($b);
    $second =~ s/[\W_]//g;

    # Check whether they are in the @standardvals list
    for (my $i = 0; $i <= $#standardvals; $i++) {
	my $firstinlist = ($first =~ /^$standardvals[$i]/);
	my $secondinlist = ($second =~ /^$standardvals[$i]/);
	if (($firstinlist) && (!$secondinlist)) {return -1};
	if (($secondinlist) && (!$firstinlist)) {return 1};
	if (($firstinlist) && ($secondinlist)) {last};
    }
	
    # None of the search terms in the list, compare the standard-formed 
    # strings
    my $compare = ( normalizename($first) cmp normalizename($second) );
    if ($compare != 0) {return $compare};

    # No other criteria fullfilled, compare the original input strings
    return $a cmp $b;
}

# Take driver/pid arguments and generate a Perl data structure for the
# Perl filter scripts. Sort the options and enumerated choices so that
# they get presented more nicely on frontends which do not sort by
# themselves

sub getdat {
    my ($this, $drv, $poid) = @_;

    my %dat;			# Our purpose in life...

    # Generate Perl data structure from database
    my $VAR1;
    eval (`$bindir/foomatic-combo-xml -d '$drv' -p '$poid' -l '$libdir' | $bindir/foomatic-perl-data -C`) ||
	die ("Could not run \"foomatic-combo-xml\"/" .
	     "\"foomatic-perl-data\"!");
    %dat = %{$VAR1};

    # Funky one-at-a-time cache thing
    $this->{'dat'} = \%dat;

    # We do some additional stuff which is very awkward to implement in C
    # now, so we do it here

    $this->sortoptions();
    $this->generalentries();

    return \%dat;
}

sub getdatfromppd {

    my ($this, $ppdfile) = @_;

    my $dat = ppdtoperl($ppdfile);
    
    if (!defined($dat)) {
	die ("Unable to open PPD file $ppdfile\n");
    }

    $this->{'dat'} = $dat;

    # Some clean-up
    $this->generalentries();

}

sub ppdtoperl {

    # Build a Perl data structure of the printer/driver options

    my ($ppdfile) = @_;

    # Load the PPD file and build a data structure for the renderer's
    # command line and the options
    open PPD, ($ppdfile !~ /\.gz$/i ? "< $ppdfile" : 
	       "$sysdeps->{'gzip'} -cd $ppdfile |") or return undef;

    my $dat = {};              # data structure for the options
    my $currentargument = "";  # We are currently reading this argument
    my $currentgroup = "";     # We are currently in this group/subgroup
    my @currentgrouptrans;     # Translation/long name for group/subgroup
    my $isfoomatic = 0;        # Do we have a Foomatic PPD?

    # If we have an old Foomatic 2.0.x PPD file, read its built-in Perl
    # data structure into @datablob and the default values in %ppddefaults
    # Then delete the $dat structure, replace it by the one "eval"ed from
    # @datablob, and correct the default settings according to the ones of
    # the main PPD structure
    my @datablob;

    # Parse the PPD file
    while(<PPD>) {
	# foomatic-rip should also work with PPD file downloaded under
	# Windows.
	undossify();
	# Parse keywords
	if (m!^\*ShortNickName:\s*\"(.*)$!) {
	    # "*ShortNickName: <code>"
	    my $line = $1;
	    # Store the value
	    # Code string can have multiple lines, read all of them
	    my $cmd = "";
	    while ($line !~ m!\"!) {
		if ($line =~ m!&&$!) {
		    # line continues in next line
		    $cmd .= substr($line, 0, -2);
		} else {
		    # line ends here
		    $cmd .= "$line\n";
		}
		# Read next line
		$line = <PPD>;
		chomp $line;
	    }
	    $line =~ m!^([^\"]*)\"!;
	    $cmd .= $1;
	    $dat->{'makemodel'} = unhtmlify($cmd);
	    $dat->{'makemodel'} =~ s/^([^,]+),.*$/$1/;
	    # The following fields are only valid for Foomatic PPDs
	    # they will be deleted when it turns out that this PPD
	    # is not a Foomatic PPD.
	    if ($dat->{'makemodel'} =~ /^(\S+)\s+(\S.*)$/) {
		$dat->{'make'} = $1;
		$dat->{'model'} = $2;
	    }
	} elsif (m!^\*FoomaticIDs:\s*(\S+)\s+(\S+)\s*$!) {
	    # "*FoomaticIDs: <printer ID> <driver ID>"
	    my $id = $1;
	    my $driver = $2;
	    # Store the values
	    $dat->{'id'} = $id;
	    $dat->{'driver'} = $driver;
	    $isfoomatic = 1;
	} elsif (m!^\*FoomaticRIPPostPipe:\s*\"(.*)$!) {
	    # "*FoomaticRIPPostPipe: <code>"
	    my $line = $1;
	    # Store the value
	    # Code string can have multiple lines, read all of them
	    my $cmd = "";
	    while ($line !~ m!\"!) {
		if ($line =~ m!&&$!) {
		    # line continues in next line
		    $cmd .= substr($line, 0, -2);
		} else {
		    # line ends here
		    $cmd .= "$line\n";
		}
		# Read next line
		$line = <PPD>;
		chomp $line;
	    }
	    $line =~ m!^([^\"]*)\"!;
	    $cmd .= $1;
	    $dat->{'postpipe'} = unhtmlify($cmd);
	} elsif (m!^\*FoomaticRIPCommandLine:\s*\"(.*)$!) {
	    # "*FoomaticRIPCommandLine: <code>"
	    my $line = $1;
	    # Store the value
	    # Code string can have multiple lines, read all of them
	    my $cmd = "";
	    while ($line !~ m!\"!) {
		if ($line =~ m!&&$!) {
		    # line continues in next line
		    $cmd .= substr($line, 0, -2);
		} else {
		    # line ends here
		    $cmd .= "$line\n";
		}
		# Read next line
		$line = <PPD>;
		chomp $line;
	    }
	    $line =~ m!^([^\"]*)\"!;
	    $cmd .= $1;
	    $dat->{'cmd'} = unhtmlify($cmd);
	} elsif (m!^\*CustomPageSize\s+True:\s*\"(.*)$!) {
	    # "*CustomPageSize True: <code>"
	    my $setting = "Custom";
	    my $translation = "Custom Size";
	    my $line = $1;
	    # Make sure that the argument is in the data structure
	    checkarg ($dat, "PageSize");
	    checkarg ($dat, "PageRegion");
	    # Make sure that the setting is in the data structure
	    checksetting ($dat, "PageSize", $setting);
	    checksetting ($dat, "PageRegion", $setting);
	    $dat->{'args_byname'}{'PageSize'}{'vals_byname'}{$setting}{'comment'} = $translation;
	    $dat->{'args_byname'}{'PageRegion'}{'vals_byname'}{$setting}{'comment'} = $translation;
	    # Store the value
	    # Code string can have multiple lines, read all of them
	    my $code = "";
	    while ($line !~ m!\"!) {
		if ($line =~ m!&&$!) {
		    # line continues in next line
		    $code .= substr($line, 0, -2);
		} else {
		    # line ends here
		    $code .= "$line\n";
		}
		# Read next line
		$line = <PPD>;
		chomp $line;
	    }
	    $line =~ m!^([^\"]*)\"!;
	    $code .= $1;
	    if ($code !~ m!^%% FoomaticRIPOptionSetting!m) {
		$dat->{'args_byname'}{'PageSize'}{'vals_byname'}{$setting}{'driverval'} = $code;
		$dat->{'args_byname'}{'PageRegion'}{'vals_byname'}{$setting}{'driverval'} = $code;
	    }
	} elsif (m!^\*Open(Sub|)Group:\s*([^\s/]+)(/(.*)|)$!) {
	    # "*Open[Sub]Group: <group>[/<translation>]
	    my $group = $2;
	    my $grouptrans = $4;
	    if (!$grouptrans) {
		$grouptrans = longname($group);
	    }
	    if ($currentgroup) {
		$currentgroup .= "/";
	    }
	    $currentgroup .= $group;
	    push(@currentgrouptrans, $grouptrans);
	} elsif (m!^\*Close(Sub|)Group:\s*([^\s/]+)$!) {
	    # "*Close[Sub]Group: <group>"
	    my $group = $2;
	    $currentgroup =~ s!$group$!!;
	    $currentgroup =~ s!/$!!;
	    pop(@currentgrouptrans);
	} elsif (m!^\*(JCL|)OpenUI\s+\*([^:]+):\s*(\S+)\s*$!) {
	    # "*[JCL]OpenUI *<option>[/<translation>]: <type>"
	    my $argnametrans = $2;
	    my $argtype = $3;
	    my $argname;
	    my $translation = "";
	    if ($argnametrans =~ m!^([^:/\s]+)/([^:]*)$!) {
		$argname = $1;
		$translation = $2;
	    } else {
		$argname = $argnametrans;
	    }
	    # Make sure that the argument is in the data structure
	    checkarg ($dat, $argname);
	    # Store the values
	    $dat->{'args_byname'}{$argname}{'comment'} = $translation;
	    $dat->{'args_byname'}{$argname}{'group'} = $currentgroup;
	    @{$dat->{'args_byname'}{$argname}{'grouptrans'}} =
		@currentgrouptrans;
	    # Set the argument type only if not defined yet, a
	    # definition in "*FoomaticRIPOption" has priority
	    if (!defined($dat->{'args_byname'}{$argname}{'type'})) {
		if ($argtype eq "PickOne") {
		    $dat->{'args_byname'}{$argname}{'type'} = 'enum';
		} elsif ($argtype eq "PickMany") {
		    $dat->{'args_byname'}{$argname}{'type'} = 'pickmany';
		} elsif ($argtype eq "Boolean") {
		    $dat->{'args_byname'}{$argname}{'type'} = 'bool';
		}
	    }
	    # Mark in which argument we are currently, so that we can find
	    # the entries for the choices
	    $currentargument = $argname;
	} elsif (m!^\*(JCL|)CloseUI:\s+\*([^:/\s]+)\s*$!) {
	    next if !$currentargument;
	    # "*[JCL]CloseUI *<option>"
	    my $argname = $2;
	    # Unmark the current argument to do not mis-interpret any 
	    # keywords as choices
	    $currentargument = "";
	} elsif ((m!^\*FoomaticRIPOption ([^/:\s]+):\s*(\S+)\s+(\S+)\s+(\S)\s*$!) ||
		 (m!^\*FoomaticRIPOption ([^/:\s]+):\s*(\S+)\s+(\S+)\s+(\S)\s+(\S+)\s*$!)){
	    next if !$currentargument;
	    # "*FoomaticRIPOption <option>: <type> <style> <spot> [<order>]"
	    # <order> only used for 1-choice enum options
	    my $argname = $1;
	    my $argtype = $2;
	    my $argstyle = $3;
	    my $spot = $4;
	    my $order = $5;
	    # Make sure that the argument is in the data structure
	    checkarg ($dat, $argname);
	    # Store the values
	    $dat->{'args_byname'}{$argname}{'type'} = $argtype;
	    if ($argstyle eq "PS") {
		$dat->{'args_byname'}{$argname}{'style'} = 'G';
	    } elsif ($argstyle eq "CmdLine") {
		$dat->{'args_byname'}{$argname}{'style'} = 'C';
	    } elsif ($argstyle eq "JCL") {
		$dat->{'args_byname'}{$argname}{'style'} = 'J';
		$dat->{'jcl'} = 1;
		$dat->{'pjl'} = 1;
	    } elsif ($argstyle eq "Composite") {
		$dat->{'args_byname'}{$argname}{'style'} = 'X';
	    }
	    $dat->{'args_byname'}{$argname}{'spot'} = $spot;
	    # $order only defined here for 1-choice enum options
	    if ($order) {
		$dat->{'args_byname'}{$argname}{'order'} = $order;
	    }
	} elsif (m!^\*FoomaticRIPOptionPrototype\s+([^/:\s]+):\s*\"(.*)$!) {
	    next if !$currentargument;
	    # "*FoomaticRIPOptionPrototype <option>: <code>"
	    # Used for numerical options only
	    my $argname = $1;
	    my $line = $2;
	    # Make sure that the argument is in the data structure
	    checkarg ($dat, $argname);
	    # Store the value
	    # Code string can have multiple lines, read all of them
	    my $proto = "";
	    while ($line !~ m!\"!) {
		if ($line =~ m!&&$!) {
		    # line continues in next line
		    $proto .= substr($line, 0, -2);
		} else {
		    # line ends here
		    $proto .= "$line\n";
		}
		# Read next line
		$line = <PPD>;
		chomp $line;
	    }
	    $line =~ m!^([^\"]*)\"!;
	    $proto .= $1;
	    $dat->{'args_byname'}{$argname}{'proto'} = unhtmlify($proto);
	} elsif (m!^\*FoomaticRIPOptionRange\s+([^/:\s]+):\s*(\S+)\s+(\S+)\s*$!) {
	    next if !$currentargument;
	    # "*FoomaticRIPOptionRange <option>: <min> <max>"
	    # Used for numerical options only
	    my $argname = $1;
	    my $min = $2;
	    my $max = $3;
	    # Make sure that the argument is in the data structure
	    checkarg ($dat, $argname);
	    # Store the values
	    $dat->{'args_byname'}{$argname}{'min'} = $min;
	    $dat->{'args_byname'}{$argname}{'max'} = $max;
	} elsif (m!^\*OrderDependency:\s*(\S+)\s+(\S+)\s+\*([^:/\s]+)\s*$!) {
	    next if !$currentargument;
	    # "*OrderDependency: <order> <section> *<option>"
	    my $order = $1;
	    my $section = $2;
	    my $argname = $3;
	    # Make sure that the argument is in the data structure
	    checkarg ($dat, $argname);
	    # Store the values
	    $dat->{'args_byname'}{$argname}{'order'} = $order;
	    $dat->{'args_byname'}{$argname}{'section'} = $section;
	} elsif (m!^\*Default([^/:\s]+):\s*([^/:\s]+)\s*$!) {
	    next if !$currentargument;
	    # "*Default<option>: <value>"
	    my $argname = $1;
	    my $default = $2;
	    # Make sure that the argument is in the data structure
	    checkarg ($dat, $argname);
	    # Store the value
	    $dat->{'args_byname'}{$argname}{'default'} = $default;
	} elsif (m!^\*$currentargument\s+([^:]+):\s*\"(.*)$!) {
	    next if !$currentargument;
	    # "*<option> <choice>[/<translation>]: <code>"
	    my $settingtrans = $1;
	    my $line = $2;
	    my $translation = "";
	    if ($settingtrans =~ m!^([^:/\s]+)/([^:]*)$!) {
		$setting = $1;
		$translation = $2;
	    } else {
		$setting = $settingtrans;
	    }
	    # Make sure that the argument is in the data structure
	    checkarg ($dat, $currentargument);
	    # Make sure that the setting is in the data structure (enum
	    # options)
	    my $bool =
		($dat->{'args_byname'}{$currentargument}{'type'} eq 'bool');
	    if ($bool) {
		if (lc($setting) eq "true") {
		    if (!$dat->{'args_byname'}{$currentargument}{'comment'}) {
			$dat->{'args_byname'}{$currentargument}{'comment'} =
			    $translation;
		    }
		    $dat->{'args_byname'}{$currentargument}{'comment_true'} =
			$translation;
		} else {
		    $dat->{'args_byname'}{$currentargument}{'comment_false'} =
			$translation;
		}
	    } else {
		checksetting ($dat, $currentargument, $setting);
		$dat->{'args_byname'}{$currentargument}{'vals_byname'}{$setting}{'comment'} = $translation;
	    }
	    # Store the value
	    # Code string can have multiple lines, read all of them
	    my $code = "";
	    while ($line !~ m!\"!) {
		if ($line =~ m!&&$!) {
		    # line continues in next line
		    $code .= substr($line, 0, -2);
		} else {
		    # line ends here
		    $code .= "$line\n";
		}
		# Read next line
		$line = <PPD>;
		chomp $line;
	    }
	    $line =~ m!^([^\"]*)\"!;
	    $code .= $1;
	    if ($code !~ m!^%% FoomaticRIPOptionSetting!) {
		if ($bool) {
		    if (lc($setting) eq "true") {
			$dat->{'args_byname'}{$currentargument}{'proto'} =
			    $code;
		    } else {
			$dat->{'args_byname'}{$currentargument}{'protof'} =
			    $code;
		    }
		} else {
		    $dat->{'args_byname'}{$currentargument}{'vals_byname'}{$setting}{'driverval'} = $code;
		}
	    }
	} elsif ((m!^\*FoomaticRIPOptionSetting\s+([^/:=\s]+)=([^/:=\s]+):\s*\"(.*)$!) ||
		 (m!^\*FoomaticRIPOptionSetting\s+([^/:=\s]+):\s*\"(.*)$!)) {
	    next if !$currentargument;
	    # "*FoomaticRIPOptionSetting <option>[=<choice>]: <code>"
	    # For boolean options <choice> is not given
	    my $argname = $1;
	    my $setting = $2;
	    my $line = $3;
	    my $bool = 0;
	    if (!$line) {
		$line = $setting;
		$bool = 1;
	    }
	    # Make sure that the argument is in the data structure
	    checkarg ($dat, $argname);
	    # Make sure that the setting is in the data structure (enum
	    # options)
	    if (!$bool) {
		checksetting ($dat, $argname, $setting);
		# Make sure that this argument has a default setting, even
		# if none is defined in this PPD file
		if (!$dat->{'args_byname'}{$argname}{'default'}) {
		    $dat->{'args_byname'}{$argname}{'default'} = $setting;
		}
	    }
	    # Store the value
	    # Code string can have multiple lines, read all of them
	    my $code = "";
	    while ($line !~ m!\"!) {
		if ($line =~ m!&&$!) {
		    # line continues in next line
		    $code .= substr($line, 0, -2);
		} else {
		    # line ends here
		    $code .= "$line\n";
		}
		# Read next line
		$line = <PPD>;
		chomp $line;
	    }
	    $line =~ m!^([^\"]*)\"!;
	    $code .= $1;
	    if ($bool) {
		$dat->{'args_byname'}{$argname}{'proto'} = unhtmlify($code);
	    } else {
		$dat->{'args_byname'}{$argname}{'vals_byname'}{$setting}{'driverval'} = unhtmlify($code);
	    }
	} elsif (m!^\*JCL(Begin|ToPSInterpreter|End):\s*\"(.*)$!) {
	    # "*JCL(Begin|ToPSInterpreter|End): <code>"
	    # The printer supports PJL/JCL when there is such a line 
	    $dat->{'jcl'} = 1;
	    $dat->{'pjl'} = 1;
	    $item = $1;
	    $line = $2;
	    # Store the value
	    # Code string can have multiple lines, read all of them
	    my $code = "";
	    while ($line !~ m!\"!) {
		if ($line =~ m!&&$!) {
		    # line continues in next line
		    $code .= substr($line, 0, -2);
		} else {
		    # line ends here
		    $code .= "$line\n";
		}
		# Read next line
	    $line = <PPD>;
		chomp $line;
	    }
	    $line =~ m!^([^\"]*)\"!;
	    $code .= $1;
	    if ($item eq 'Begin') {
		$jclbegin = unhexify($code);
	    } elsif ($item eq 'ToPSInterpreter') {
		$jcltointerpreter = unhexify($code);
	    } elsif ($item eq 'End') {
		$jclend = unhexify($code);
	    }
	} elsif (m!^\*\% COMDATA \#(.*)$!) {
	    # If we have an old Foomatic 2.0.x PPD file, collect its Perl 
	    # data
	    push (@datablob, $1);
	}
    }
    close PPD;

    # Remove make and model fields when we don't have a Foomatic PPD file
    if (!$isfoomatic) {
	$dat->{'make'} = undef;
	$dat->{'model'} = undef;
    }

    # If we have an old Foomatic 2.0.x PPD file use its Perl data structure
    if ($#datablob >= 0) {
	my $VAR1;
	if (eval join('',@datablob)) {
	    # Overtake default settings from the main structure of the
	    # PPD file
	    for my $arg (@{$dat->{'args'}}) {
		if ($arg->{'default'}) {
		    $VAR1->{'argsbyname'}{$arg->{'name'}}{'default'} = 
			$arg->{'default'};
		}
	    }
	    undef $dat;
	    $dat = $VAR1;
	    $dat->{'jcl'} = $dat->{'pjl'};
	} else {
	    # Perl structure broken
	    warn "\nUnable to evaluate datablob, print jobs may come " .
		"out incorrectly or not at all.\n\n";
	}
    }

    return $dat;
}

sub ppdgetdefaults {

    # Read a PPD and get only the defaults and the postpipe.
    my ($this, $ppdfile) = @_;
    
    # Open the PPD file
    open PPD, ($ppdfile !~ /\.gz$/i ? "< $ppdfile" : 
	       "$sysdeps->{'gzip'} -cd $ppdfile |") or do {
	die ("Unable to open PPD file $ppdfile\n");
    };

    # We don't read the "COMDATA" lines of old Foomatic 2.0.x PPD files
    # here, because the defaults in the main PPD structure have priority.
    while(<PPD>) {
	# foomatic-rip should also work with PPD file downloaded under
	# Windows.
	undossify();
	# Parse keywords
	if (m!^\*FoomaticRIPPostPipe:\s*\"(.*)$!) {
	    # "*FoomaticRIPPostPipe: <code>"
	    my $line = $1;
	    # Store the value
	    # Code string can have multiple lines, read all of them
	    my $cmd = "";
	    while ($line !~ m!\"!) {
		if ($line =~ m!&&$!) {
		    # line continues in next line
		    $cmd .= substr($line, 0, -2);
		} else {
		    # line ends here
		    $cmd .= "$line\n";
		}
		# Read next line
		$line = <PPD>;
		chomp $line;
	    }
	    $line =~ m!^([^\"]*)\"!;
	    $cmd .= $1;
	    $this->{'dat'}{'postpipe'} = unhtmlify($cmd);
	} elsif (m!^\*Default([^/:\s]+):\s*([^/:\s]+)\s*$!) {
	    # "*Default<option>: <value>"
	    my $argname = $1;
	    my $default = $2;
	    if (defined($this->{'dat'}{'args_byname'}{$argname})) {
		# Store the value
		$this->{'dat'}{'args_byname'}{$argname}{'default'} =
		    $default;
	    }
	}
    }
    close PPD;
}

sub ppdsetdefaults {

    my ($this, $ppdfile) = @_;
    
    my @ppdlines;
    my $ppd;

    # Load the complete PPD file into memory but remove the postpipe
    #system "cat $ppdfile";
    open PPD, ($ppdfile !~ /\.gz$/i ? "< $ppdfile" : 
	       "$sysdeps->{'gzip'} -cd $ppdfile |") or
	       die ("Unable to open PPD file $ppdfile\n");
    while (my $line = <PPD>) {
	if ($line =~ m!^\*FoomaticRIPPostPipe:\s*\"(.*)$!) {
	    # "*FoomaticRIPPostPipe: <code>"
	    # Code string can have multiple lines, read all of them
	    my $line = $1;
	    while ($line !~ m!\"!) {
		# Read next line
		$line = <PPD>;
	    }
	    # We also have to remove the "*End" line
	    $line = <PPD>;
	    if ($line !~ /^\*End/) {
		push(@ppdlines, $line);
	    }
	} else {
	    push(@ppdlines, $line);
	}
    }
    close PPD;
    $ppd = join('', @ppdlines);

    # If the settings for "PageSize" and "PageRegion" are different,
    # set the one for "PageRegion" to the one for "PageSize".
    if ($this->{'dat'}{'args_byname'}{'PageSize'}{'default'} ne
	$this->{'dat'}{'args_byname'}{'PageRegion'}{'default'}) {
	$this->{'dat'}{'args_byname'}{'PageRegion'}{'default'} =
	    $this->{'dat'}{'args_byname'}{'PageSize'}{'default'}
    }

    # Set the defaults in the PPD file to the current default
    # settings in the data structure
    for my $arg (@{$this->{'dat'}{'args'}}) {
	if (defined($arg->{'default'})) {
	    my $name = $arg->{'name'};
	    my $def = $arg->{'default'};
	    if ($arg->{'type'} eq 'bool') {
		if ((lc($def) eq '1')   || (lc($def) eq 'on') || 
		    (lc($def) eq 'yes') || (lc($def) eq 'true')) {
		    $def='True';
		} elsif ((lc($def) eq '0')  || (lc($def) eq 'off') || 
			 (lc($def) eq 'no') || (lc($def) eq 'false')) {
		    $def='False';
		}
	    }
	    $ppd =~ s!^(\*Default$name:\s*)([^/:\s\r]+)(\s*\r?)$!$1$def$3!m;
	}
    }

    # Update the postpipe
    if ($this->{'dat'}{'postpipe'}) {
	my $header = "*FoomaticRIPPostPipe";
	my $code = $this->{'dat'}{'postpipe'};
	my $postpipestr = ripdirective($header, $code) . "\n";
	if ($postpipestr =~ /\n.*\n/s) {
	    $postpipestr .= "*End\n";
	}
	#$ppd =~ s/(\*PPD[^a-zA-Z0-9].*\n)/$1$postpipestr/s;
	$ppd =~ s/((\r\n|\n\r|\r|\n))/$1$postpipestr/s;
    }
    
    # Write back the modified PPD file
    open PPD, ($ppdfile !~ /\.gz$/i ? "> $ppdfile" : 
	       "| $sysdeps->{'gzip'} > $ppdfile") or
	die ("Unable to open PPD file $ppdfile for writing\n");
    print PPD $ppd;
    close PPD;
    
}

# Some helper functions for reading the PPD file

sub unhtmlify {
    # Replace HTML/XML entities by the original characters
    my $str = $_[0];
    $str =~ s/\&apos;/\'/g;
    $str =~ s/\&quot;/\"/g;
    $str =~ s/\&gt;/\>/g;
    $str =~ s/\&lt;/\</g;
    $str =~ s/\&amp;/\&/g;
    return $str;
}

sub unhexify {
    # Replace hex notation for unprintable characters in PPD files
    # by the actual characters ex: "<0A>" --> chr(hex("0A"))
    my ($input) = @_;
    my $output = "";
    my $hexmode = 0;
    my $firstdigit = "";
    for (my $i = 0; $i < length($input); $i ++) {
	my $c = substr($input, $i, 1);
	if ($hexmode) {
	    if ($c eq ">") {
		# End of hex string
		$hexmode = 0;
	    } elsif ($c =~ /^[0-9a-fA-F]$/) {
		# Hexadecimal digit, two of them give a character
		if ($firstdigit ne "") {
		    $output .= chr(hex("$firstdigit$c"));
		    $firstdigit = "";
		} else {
		    $firstdigit = $c;
		}
	    }
	} else {
	    if ($c eq "<") {
		# Beginning of hex string
		$hexmode = 1;
	    } else {
		# Normal character
		$output .= $c;
	    }
	}
    }
    return $output;
}

sub undossify {
    # Remove "dossy" line ends ("\r\n") from a string
    my ($str) = @_;
    $str =~ s/\r\n/\n/gs;
    $str =~ s/\r$//s;
    return $str;
}

sub checkarg {
    # Check if there is already an argument record $argname in $dat, if
    # create one
    my ($dat, $argname) = @_;
    return if defined($dat->{'args_byname'}{$argname});
    # argument record
    my $rec;
    $rec->{'name'} = $argname;
    # Insert record in 'args' array for browsing all arguments
    push(@{$dat->{'args'}}, $rec);
    # 'args_byname' hash for looking up arguments by name
    $dat->{'args_byname'}{$argname} = $dat->{'args'}[$#{$dat->{'args'}}];
    # Default execution style is 'G' (PostScript) since all arguments for
    # which we don't find "*Foomatic..." keywords are usual PostScript
    # options
    $dat->{'args_byname'}{$argname}{'style'} = 'G';
    # Default prototype for code to insert, used by enum options
    $dat->{'args_byname'}{$argname}{'proto'} = '%s';
}

sub checksetting {
    # Check if there is already an argument record $argname in $dat, if
    # create one
    my ($dat, $argname, $setting) = @_;
    return if 
	defined($dat->{'args_byname'}{$argname}{'vals_byname'}{$setting});
    # setting record
    my $rec;
    $rec->{'value'} = $setting;
    # Insert record in 'vals' array for browsing all settings
    push(@{$dat->{'args_byname'}{$argname}{'vals'}}, $rec);
    # 'vals_byname' hash for looking up settings by name
    $dat->{'args_byname'}{$argname}{'vals_byname'}{$setting} = 
	$dat->{'args_byname'}{$argname}{'vals'}[$#{$dat->{'args_byname'}{$argname}{'vals'}}];
}

sub removearg {
    # remove the argument record $argname from $dat
    my ($dat, $argname) = @_;
    return if !defined($dat->{'args_byname'}{$argname});
    # Remove 'args_byname' hash for looking up arguments by name
    delete $dat->{'args_byname'}{$argname};
    # Remove argument itself
    for (my $i = 0; $i <= $#{$dat->{'args'}}; $i ++) {
	if ($dat->{'args'}[$i]{'name'} eq $argname) {
	    splice(@{$dat->{'args'}}, $i, 1);
	    last;
	}
    }
}

sub booltoenum {
    # Turn the boolean argument $argname from $dat to an enumerated choice
    # equivalent to the original argument
    my ($dat, $argname) = @_;
    return if !defined($dat->{'args_byname'}{$argname});
    # Argument record
    my $arg = $dat->{'args_byname'}{$argname};
    # General settings
    $arg->{'type'} = 'enum';
    my $proto = $arg->{'proto'};
    $arg->{'proto'} = '%s';
    # Choice for 'true'
    if (!defined($arg->{'name_true'})) {
	$arg->{'name_true'} = $arg->{'name'};
    }
    checksetting($dat, $argname, 'true');
    my $truechoice = $arg->{'vals_byname'}{'true'};
    $truechoice->{'comment'} = longname($arg->{'name_true'});
    $truechoice->{'driverval'} = $proto;
    # Choice for 'false'
    if (!defined($arg->{'name_false'})) {
	$arg->{'name_false'} = "no$arg->{'name'}";
    }
    checksetting($dat, $argname, 'false');
    my $truechoice = $arg->{'vals_byname'}{'false'};
    $truechoice->{'comment'} = longname($arg->{'name_false'});
    $truechoice->{'driverval'} = '';
    # Default value
    if ($arg->{'default'} eq '0') {
	$arg->{'default'} = 'false';
    } else {
	$arg->{'default'} = 'true';
    }
}

sub checkoptionvalue {

    ## This function checks whether a given value is valid for a given
    ## option. If yes, it returns a cleaned value (e. g. always 0 or 1
    ## for boolean options), otherwise "undef". If $forcevalue is set,
    ## we always determine a corrected value to insert (we never return
    ## "undef").

    # Is $value valid for the option named $argname?
    my ($dat, $argname, $value, $forcevalue) = @_;

    # Record for option $argname
    my $arg = $dat->{'args_byname'}{$argname};

    if ($arg->{'type'} eq 'bool') {
	if ((lc($value) eq 'true') ||
	    (lc($value) eq 'on') ||
	    (lc($value) eq 'yes') ||
	    (lc($value) eq '1')) {
	    return 1;
	} elsif ((lc($value) eq 'false') ||
		 (lc($value) eq 'off') ||
		 (lc($value) eq 'no') ||
		 (lc($value) eq '0')) {
	    return 0;
	} elsif ($forcevalue) {
	    # This maps Unknown to mean False.  Good?  Bad?
	    # It was done so in Foomatic 2.0.x, too.
	    my $name = $arg->{'name'};
	    return 0;
	}
    } elsif ($arg->{'type'} eq 'enum') {
	if (defined($arg->{'vals_byname'}{$value})) {
	    return $value;
	} elsif ((($arg->{'name'} eq "PageSize") ||
		  ($arg->{'name'} eq "PageRegion")) &&
		 (defined($arg->{'vals_byname'}{'Custom'})) &&
		 ($value =~ m!^Custom\.([\d\.]+)x([\d\.]+)([A-Za-z]*)$!)) {
	    # Custom paper size
	    return $value;
	} elsif ($forcevalue) {
	    # wtf!?  that's not a choice!
	    my $name = $arg->{'name'};
	    # Return the first entry of the list
	    my $firstentry = $arg->{'vals'}[0]{'value'};
	    return $firstentry;
	}
    } elsif (($arg->{'type'} eq 'int') ||
	     ($arg->{'type'} eq 'float')) {
	if (($value <= $arg->{'max'}) &&
	    ($value >= $arg->{'min'})) {
	    return $value;
	} elsif ($forcevalue) {
	    my $name = $arg->{'name'};
	    my $newvalue;
	    if ($value > $arg->{'max'}) {
		$newvalue = $arg->{'max'}
	    } elsif ($value < $arg->{'min'}) {
		$newvalue = $arg->{'min'}
	    }
	    return $newvalue;
	}
    }
    return undef;
}

sub checkoptions {

    ## Let the values of a boolean option being 0 or 1 instead of
    ## "True" or "False", range-check the defaults of all options and
    ## issue warnings if the values are not valid

    # Option set to be examined
    my ($dat, $optionset) = @_;

    for $arg (@{$dat->{'args'}}) {
	if (defined($arg->{$optionset})) {
	    $arg->{$optionset} =
		checkoptionvalue
		($dat, $arg->{'name'}, $arg->{$optionset}, 1);
	}
    }

    # If the settings for "PageSize" and "PageRegion" are different,
    # set the one for "PageRegion" to the one for "PageSize" and issue
    # a warning.
    if ($dat->{'args_byname'}{'PageSize'}{$optionset} ne
	$dat->{'args_byname'}{'PageRegion'}{$optionset}) {
	$dat->{'args_byname'}{'PageRegion'}{$optionset} =
	    $dat->{'args_byname'}{'PageSize'}{$optionset};
    }
}

# If the PageSize or PageRegion was changed, also change the other

sub syncpagesize {
    
    # Name and value of the option we set, and the option set where we
    # did the change
    my ($dat, $name, $value, $optionset) = @_;

    # Don't do anything if we were called with an option other than
    # "PageSize" or "PageRegion"
    return if (($name ne "PageSize") && ($name ne "PageRegion"));
    
    # Don't do anything if not both "PageSize" and "PageRegion" exist
    return if ((!defined($dat->{'args_byname'}{'PageSize'})) ||
	       (!defined($dat->{'args_byname'}{'PageRegion'})));
    
    my $dest;
    
    # "PageSize" --> "PageRegion"
    if ($name eq "PageSize") {
	$dest = "PageRegion";
    }
    
    # "PageRegion" --> "PageSize"
    if ($name eq "PageRegion") {
	$dest = "PageSize";
    }
    
    # Do it!
    if (my $val=valbyname($dat->{'args_byname'}{$dest}, $value)) {
	# Standard paper size
	$dat->{'args_byname'}{$dest}{$optionset} = $val->{'value'};
    } elsif (my $val=valbyname($dat->{'args_byname'}{$dest}, "Custom")) {
	# Custom paper size
	$dat->{'args_byname'}{$dest}{$optionset} = $value;
    }
}

sub sortoptions {

    my ($this) = @_;

    # The following stuff is very awkward to implement in C, so we do
    # it here.

    # Sort options with "sortargs" function
    my @sortedarglist = sort sortargs @{$this->{'dat'}{'args'}};
    @{$this->{'dat'}{'args'}} = @sortedarglist;

    # Sort values of enumerated options with "sortvals" function
    for my $arg (@{$this->{'dat'}{'args'}}) {
       	my @sortedvalslist = sort sortvals keys(%{$arg->{'vals_byname'}});
	@{$arg->{'vals'}} = ();
	for my $i (@sortedvalslist) {
	    my $val = $arg->{'vals_byname'}{$i};
	    push (@{$arg->{'vals'}}, $val);
	}
    }

}

sub generalentries {

    my ($this) = @_;

    $this->{'dat'}{'compiled-at'} = localtime(time());
    $this->{'dat'}{'timestamp'} = time();

    my $user = `whoami`; chomp $user;
    my $host = `hostname`; chomp $host;

    $this->{'dat'}{'compiled-by'} = "$user\@$host";

}



#####################
# PPD generation
#

# member( $a, @b ) returns 1 if $a is in @b, 0 otherwise.
sub member { my $e = shift; foreach (@_) { $e eq $_ and return 1 } 0 };

# Return a generic Adobe-compliant PPD for the "foomatic-rip" filter script
# for all spoolers.  Built from the standard data; you must call getdat()
# first.
sub getppd {

    # If $members_in_subgroup is set, the member options of a composite
    # option go into a subgroup of the group where the composite option
    # is located. Otherwise the member options go into a new main group
    my ($db, $members_in_subgroup) = @_;

    die "you need to call getdat first!\n" 
	if (!defined($db->{'dat'}));

    # The Perl data structure of the current printer/driver combo.
    my $dat = $db->{'dat'};

    my @optionblob; # Lines for command line and options in the PPD file

    # Insert the printer/driver IDs and the command line prototype
    # right before the option descriptions

    push(@optionblob, "*FoomaticIDs: $dat->{'id'} $dat->{'driver'}\n");
    my $header = "*FoomaticRIPCommandLine";
    my $cmdline = $dat->{'cmd'};
    my $cmdlinestr = ripdirective($header, $cmdline);
    push(@optionblob, "$cmdlinestr\n");
    if ($cmdlinestr =~ /\n/s) {
	push(@optionblob, "*End\n");
    }

    # Search for composite options and prepare the member options
    # of the found composite options
    for $arg (@{$dat->{'args'}}) {
	# Here we are only interested in composite options, skip the others
	next if $arg->{'style'} ne 'X';
	my $name = $arg->{'name'};
	my $com  = $arg->{'comment'};
	my $group = $arg->{'group'};
	my $order = $arg->{'order'};
	my $section = $arg->{'section'};

	# Set default for missing section value
	if (!defined($section)) {$arg->{'section'} = "AnySetup";}

	# Set default for missing tranaslation/longname
	if (!$com) {$com = longname($name);}

	my @members;

	# Go through all choices of the composite option to find its
	# member options
	for my $v (@{$arg->{'vals'}}) {
	    my @settings = split(/\s+/s, $v->{'driverval'});
	    for my $s (@settings) {
		if (($s =~ /^([^=]+)=/) ||
		    ($s =~ /^[Nn][Oo]([^=]+)$/) ||
		    ($s =~ /^([^=]+)$/)) {
		    my $m = $1;
		    # Does the found member exist for this printer/driver
		    # combo?
		    if (defined($dat->{'args_byname'}{$m})) {
			# Add it to the list of found member options
			if (!member($m, @members)) {
			    push(@members, $1);
			}
			# Clean up entries for boolean options
			if ($s !~ /=/) {
			    if ($s =~ /^[Nn][Oo]$m$/) {
				$v->{'driverval'} =~
				    s/(^|\s)$s(\s|$)/$1$m=false$2/;
			    } else {
				$v->{'driverval'} =~ 
				    s/(^|\s)$s(\s|$)/$1$m=true$2/;
			    }
			}
		    } else {
			# Remove it from the choice of the composite
			# option
			$v->{'driverval'} =~ s/$s\s*//;
			$v->{'driverval'} =~ s/\s*$//;
		    }
		}
	    }
	}
	# Set group of member options and add a "From<Composite>" choice
	# which will be the default. Make also sure that the composite
	# option will be inserted into the PostScript code before all its
	# members are inserted (by means of the section and the order
	# number).
	for my $m (@members) {
	    my $a = $dat->{'args_byname'}{$m};

	    # Convert boolean options to enumerated choice options, so
	    # that we can add the "From<Composite>" choice.
	    if ($a->{'type'} eq 'bool') {
		booltoenum($dat, $a->{'name'});
	    }

	    # If $members_in_subgroup is set, the group should be a
	    # subgroup of the group where the composite option is
	    # located, named as the composite option. Otherwise the
	    # group will get a new main group.
	    if (($members_in_subgroup) && ($group)) {
		$a->{'group'} = "$group/$name";
	    } else {
		$a->{'group'} = "$name";
	    }

	    # Determine section and order number for the composite option
	    # Order of the DSC sections of a PostScript file
	    my @sectionorder = ("JCLSetup", "Prolog", "DocumentSetup", 
				"AnySetup", "PageSetup");
	    # Set default for missing section value in member
	    if (!defined($a->{'section'})) {$a->{'section'} = "AnySetup";}
	    my $minsection;
	    for my $s (@sectionorder) {
		if (($s eq $arg->{'section'}) || ($s eq $a->{'section'})) {
		    $minsection = $s;
		    last;
		}
	    }
	    # If the current member option is in an earlier section,
	    # put also the composite option into it. Do never put the
	    # composite option into the JCL setup because in the JCL
	    # header PostScript comments are not allowed.
	    $arg->{'section'} = ($minsection ne "JCLSetup" ?
				 $minsection : "Prolog");
	    # Let the order number of the composite option be less
	    # than the order number of the current member
	    if ($arg->{'order'} >= $a->{'order'}) {
		$arg->{'order'} = $a->{'order'} - 1;
		if ($arg->{'order'} < 0) {
		    $arg->{'order'} = 0;
		}
	    }

	    # Do not add a "From<Composite>" choice to an option with only
	    # one choice
	    next if $#{$a->{'vals'}} < 1;

	    # Add "From<Composite>" choice
	    # setting record
	    my $rec;
	    $rec->{'value'} = "From$name";
	    $rec->{'comment'} = "Controlled by '$com'";
	    # We mark the driverval as invalid with a non-printable
	    # character, this means that the code to insert will be an
	    # empty string in the PPD.
	    $rec->{'driverval'} = "\x01";
	    # Insert record as the first item in the 'vals' array
	    unshift(@{$a->{'vals'}}, $rec);
	    # Update 'vals_byname' hash
	    $a->{'vals_byname'}{$rec->{'value'}} = $a->{'vals'}[0];
	    for (my $i = 1; $i <= $#{$a->{'vals'}}; $i ++) {
		$a->{'vals_byname'}{$a->{'vals'}[$i]{'value'}} =
		    $a->{'vals'}[$i];
	    }

	    # Set default to the new "From<Composite>" choice
	    $a->{'default'} = $rec->{'value'};
	}
    }
 
    # Sort options with "sortargs" function after they were re-grouped
    # due to the composite options
    my @sortedarglist = sort sortargs @{$dat->{'args'}};
    @{$dat->{'args'}} = @sortedarglist;

    # Construct the option entries for the PPD file

    my @groupstack; # In which group are we currently

    for $arg (@{$dat->{'args'}}) {
	my $name = $arg->{'name'};
	my $type = $arg->{'type'};
	my $com  = $arg->{'comment'};
	my $default = $arg->{'default'};
	my $order = $arg->{'order'};
	my $spot = $arg->{'spot'};
	my $section = $arg->{'section'};
	my $cmd = $arg->{'proto'};
	my @group = split("/", $arg->{'group'});
	my $idx = $arg->{'idx'};

	# What is the execution style of the current option? Skip options
        # of unknown execution style
	my $optstyle = ($arg->{'style'} eq 'G' ? "PS" :
			($arg->{'style'} eq 'J' ? "JCL" :
			 ($arg->{'style'} eq 'C' ? "CmdLine" :
			  ($arg->{'style'} eq 'X' ? "Composite" :
			   "Unknown"))));
	next if $optstyle eq "Unknown";

	# The command prototype should not be empty, set default
	if (!$cmd) {
	    $cmd = "%s";
	}

	# Set default for missing section value
	if (!defined($section)) {$section = "AnySetup";}

	# Set default for missing tranaslation/longname
	if (!$com) {$com = longname($name);}

	# Do we have to open or close one or more groups here?
	# No group will be opened more than once, since the options
	# are sorted to have the members of every group together

	# Only take into account the groups of options which will be
	# visible user interface options in the PPD.
	if (($type ne 'enum') || ($#{$arg->{'vals'}} > 0) ||
	    ($name eq "PageSize") || ($arg->{'style'} eq 'G')) {
	    # Find the level on which the group path of the current option
	    # (@group) differs from the group path of the last option
	    # (@groupstack).
	    my $level = 0;
	    while (($level <= $#groupstack) and
		   ($level <= $#group) and 
		   ($groupstack[$level] eq $group[$level])) {
		$level++;
	    }
	    for (my $i = $#groupstack; $i >= $level; $i--) {
		# Close this group, the current option is not member
		# of it.
		push(@optionblob,
		     sprintf("\n*Close%sGroup: %s\n",
			     ($i > 0 ? "Sub" : ""), $groupstack[$i])
		     );
		pop(@groupstack);
	    }
	    for (my $i = $level; $i <= $#group; $i++) {
		# Open this group, the current option is a member
		# of it.
		push(@optionblob,
		     sprintf("\n*Open%sGroup: %s/%s\n",
			     ($i > 0 ? "Sub" : ""), $group[$i], 
			     longname($group[$i]))
		     );
		push(@groupstack, $group[$i]);
	    }
	}

	if ($type eq 'enum') {
	    # Skip zero or one choice arguments. Do not skip "PageSize",
	    # since a PPD file without "PageSize" will break the CUPS
	    # environment and also do not skip PostScript options. For
	    # skipped options with one choice only "*Foomatic..."
	    # definitions will be used.
	    if ((1 < scalar(@{$arg->{'vals'}})) ||
		($name eq "PageSize") ||
		($arg->{'style'} eq 'G')) {

		push(@optionblob,
		     sprintf("\n*OpenUI *%s/%s: PickOne\n", $name, $com));

		if ($arg->{'style'} ne 'G') {
		    # For non-PostScript options insert line with option
		    # properties
		    push(@optionblob, sprintf
			 ("*FoomaticRIPOption %s: enum %s %s\n",
			  $name, $optstyle, $spot));
		}

		push(@optionblob,
		     sprintf("*OrderDependency: %s %s *%s\n", 
			     $order, $section, $name),
		     sprintf("*Default%s: %s\n", 
			     $name,
			     (defined($default) ? $default : 'Unknown')));

		if (!defined($default)) {
		    my $whr = sprintf("%s %s driver %s",
				      $dat->{'make'},
				      $dat->{'model'},
				      $dat->{'driver'});
		    warn "undefined default for $idx/$name on a $whr\n";
		}
		
		# If this is the page size argument; construct
		# PageRegion, ImageableArea, and PaperDimension clauses 
		# from it. Arguably this is all backwards, but what can
		# you do! ;)
		my @pageregion;
		my @imageablearea;
		my @paperdimension;

		# If we have a paper size named "Custom", or one with
		# one or both dimensions being zero, we must replace
		# this by an Adobe-complient custom paper size
		# definition.
		my $hascustompagesize = 0;

		# We take very big numbers now, to not impose linits.
		# Later, when we will have physical demensions of the
		# printers in the database.
		my $maxpagewidth = 100000;
		my $maxpageheight = 100000;

		# Start the PageRegion, ImageableArea, and PaperDimension
		# clauses
		if ($name eq "PageSize") {
		    
		    push(@pageregion,
			 "*OpenUI *PageRegion: PickOne
*OrderDependency: $order $section *PageRegion
*DefaultPageRegion: $dat->{'args_byname'}{'PageSize'}{'default'}");
		    push(@imageablearea, 
			 "*DefaultImageableArea: $dat->{'args_byname'}{'PageSize'}{'default'}");
		    push(@paperdimension, 
			 "*DefaultPaperDimension: $dat->{'args_byname'}{'PageSize'}{'default'}");
		}

		my $v;
		for $v (@{$arg->{'vals'}}) {
		    my $psstr = "";

		    if ($name eq "PageSize") {
		    
			my $value = $v->{'value'}; # in a PPD, the value 
			                           # is the PPD name...
			my $comment = $v->{'comment'};

			# Here we have to fill in the absolute sizes of the 
			# papers. We consult a table when we could not read
			# the sizes out of the choices of the "PageSize"
			# option.
			my $size = $v->{'driverval'};
			if (($size !~ /^\s*(\d+)\s+(\d+)\s*$/) &&
			    # 2 positive integers separated by whitespace
			    ($size !~ /\-dDEVICEWIDTHPOINTS\=(\d+)\s+\-dDEVICEHEIGHTPOINTS\=(\d+)/)) {
			    # "-dDEVICEWIDTHPOINTS=..."/"-dDEVICEHEIGHTPOINTS=..."
			    $size = getpapersize($value);
			} else {
			    $size = "$1 $2";
			}
			$size =~ /^\s*(\d+)\s+(\d+)\s*$/;
			my $width = $1;
			my $height = $2;
			if ($maxpagewidth < $width) {
			    $maxpagewidth = $width;
			}
			if ($maxpageheight < $height) {
			    $maxpageheight = $height;
			}
			if (($value eq "Custom") ||
			    ($width == 0) || ($height == 0)) {
			    # This page size is either named "Custom" or
			    # at least one of its dimensions is not fixed
			    # (=0), so this printer/driver combo must
			    # support custom page sizes
			    $hascustompagesize = 1;
			    # We do not add this size to the PPD file
			    # because the Adobe standard foresees a
			    # special code block in the header of the
			    # PPD file to be inserted when a custom
			    # page size is requested.
			    next;
			}
			# Determine the unprintable margins
			# Zero margins when no margin info exists
			my ($left, $right, $top, $bottom) =
			    getmargins($dat, $width, $height, $value);
			# Insert margins in "*ImageableArea" line
			push(@imageablearea,
			     "*ImageableArea $value/$comment: " . 
			     "\"$left $bottom $right $top\"");
			push(@paperdimension,
			     "*PaperDimension $value/$comment: \"$size\"");
		    }
		    my $foomaticstr = "";
		    # For PostScript options PostScript code must be 
		    # inserted, unless they are member of a composite
		    # option AND they are set to the "Controlled by
		    # '<Composite>'" choice (driverval is "\x01")
		    if (($arg->{'style'} eq 'G') &&
			($v->{'driverval'} ne "\x01")) {
			# Ghostscript argument; offer up ps for
			# insertion
			$psstr = sprintf($cmd, 
					 (defined($v->{'driverval'})
					  ? $v->{'driverval'}
					  : $v->{'value'}));
		    } else {
			# Option setting directive for Foomatic filter
			# 4 "%" because of the "sprintf" applied to it
			# In the end stay 2 "%" to have a PostScript 
			# comment
			$psstr = sprintf
			    ("%%%% FoomaticRIPOptionSetting: %s=%s",
			     $name, $v->{'value'});
			if ($v->{'driverval'} eq "\x01") {
			    # Only set the $foomaticstr when the selected
			    # choice is not the "Controlled by
			    # '<Composite>'" of a member of a collective
			    # option. Otherwise leave it out and let
			    # the value in the "FoomaticRIPOptionSetting"
			    # comment be "@<Composite>".
			    $psstr =~ s/=From/=\@/;
			    $foomaticstr = "";
			} else {
			    my $header = sprintf
				("*FoomaticRIPOptionSetting %s=%s",
				 $name, $v->{'value'});
			    my $cmdval = "";
			    $cmdval =
				sprintf($cmd,
					(defined($v->{'driverval'})
					 ? $v->{'driverval'}
					 : $v->{'value'}));
			    $foomaticstr = ripdirective($header, $cmdval) . 
				"\n";
			}
		    }
		    # Make sure that the longname/translation exists
		    if (!$v->{'comment'}) {
			$v->{'comment'} = longname($v->{'value'});
		    }
		    # Code supposed to be inserted into the PostScript
		    # data when this choice is selected.
		    push(@optionblob,
			 sprintf("*%s %s/%s: \"%s\"\n", 
				 $name, $v->{'value'}, $v->{'comment'},
				 $psstr));
		    # PostScript code is more than one line? Let an "*End"
		    # line follow
		    if ($psstr =~ /\n/s) {
			push(@optionblob, "*End\n");
		    }
		    # If we have a command line or JCL option, insert the
		    # code here. For security reasons command line snippets
		    # cannot be inserted into the "official" choice entry,
		    # otherwise the appropriate RIP filter could execute
		    # arbitrary code.
		    push(@optionblob, $foomaticstr);
		    # Stuff to insert into command line/job is more than one
		    # line? Let an "*End" line follow
		    if ($foomaticstr =~ /\n.*\n/s) {
			push(@optionblob, "*End\n");
		    }
		    # In modern PostScript interpreters "PageRegion" 
		    # and "PageSize" are the same option, so we fill 
		    # in the "PageRegion" the same
		    # way as the "PageSize" choices.
		    if ($name eq "PageSize") {
			push(@pageregion,
			     sprintf("*PageRegion %s/%s: \"%s\"", 
				     $v->{'value'}, $v->{'comment'},
				     $psstr));
			if ($psstr =~ /\n/s) {
			    push(@pageregion, "*End");
			}
		    }
		}
		
		push(@optionblob,
		     sprintf("*CloseUI: *%s\n", $name));

		if ($name eq "PageSize") {
		    # Close the PageRegion, ImageableArea, and 
		    # PaperDimension clauses
		    push(@pageregion,
			 "*CloseUI: *PageRegion");

		    my $paperdim = join("\n", 
					("", @pageregion, "", 
					 @imageablearea, "",
					 @paperdimension, ""));
		    push (@optionblob, $paperdim);

		    # Make the header entries for a custom page size
		    if ($hascustompagesize) {
			my $maxpaperdim = 
			    ($maxpageheight > $maxpagewidth ?
			     $maxpageheight : $maxpagewidth);
			# PostScript code from the example 6 in section 6.3
			# of Adobe's PPD V4.3 specification
			# http://partners.adobe.com/asn/developer/pdfs/tn/5003.PPD_Spec_v4.3.pdf
			# If the page size is an option for the command line
			# of GhostScript, let the values which where put
			# on the stack being popped and inserta comment
			# to advise the filter
			
			my $pscode;
			my $foomaticstr = "";
			if ($arg->{'style'} eq 'G') {
			    $pscode = "pop pop pop
<</PageSize [ 5 -2 roll ] /ImagingBBox null>>setpagedevice";
			} else {
			    my $a = $arg->{'vals_byname'}{'Custom'};
			    my $header = sprintf
				("*FoomaticRIPOptionSetting %s=%s",
				 $name, $a->{'value'});
			    my $cmdval =
				sprintf($cmd,
					(defined($a->{'driverval'})
					 ? $a->{'driverval'}
					 : $a->{'value'}));
			    $foomaticstr =
				ripdirective($header, $cmdval) . "\n";
			    # Stuff to insert into command line/job is more
			    # than one line? Let an "*End" line follow
			    if ($foomaticstr =~ /\n.*\n/s) {
				$foomaticstr .= "*End\n";
			    }
			    $pscode = "pop pop pop pop pop
%% FoomaticRIPOptionSetting: $name=Custom";
			}
			my ($left, $right, $top, $bottom) =
			    getmargins($dat, 0, 0, 'Custom');
			my $custompagesizeheader = 
"*HWMargins: $left $bottom $right $top
*VariablePaperSize: True
*MaxMediaWidth: $maxpaperdim
*MaxMediaHeight: $maxpaperdim
*NonUIOrderDependency: $order $section *CustomPageSize
*CustomPageSize True: \"$pscode\"
*End
${foomaticstr}*ParamCustomPageSize Width: 1 points 36 $maxpagewidth
*ParamCustomPageSize Height: 2 points 36 $maxpageheight
*ParamCustomPageSize Orientation: 3 int 0 0
*ParamCustomPageSize WidthOffset: 4 points 0 0
*ParamCustomPageSize HeightOffset: 5 points 0 0

";
			
			unshift (@optionblob, $custompagesizeheader);
		    } else {
			unshift (@optionblob,
				 "*VariablePaperSize: False\n\n");
		    }
		}
	    } elsif ((1 == scalar(@{$arg->{'vals'}})) &&
		($arg->{'style'} ne 'G')) {
		# Enumerated choice option with one single choice

		# For non-PostScript options insert line with option
		# properties
		my $v = $arg->{'vals'}[0];
		my $header = sprintf
		    ("*FoomaticRIPOptionSetting %s=%s",
		     $name, $v->{'value'});
		my $cmdval =
		    sprintf($cmd,
			    (defined($v->{'driverval'})
			     ? $v->{'driverval'}
			     : $v->{'value'}));
		my $foomaticstr = ripdirective($header, $cmdval) . "\n";
		# Stuff to insert into command line/job is more
		# than one line? Let an "*End" line follow
		if ($foomaticstr =~ /\n.*\n/s) {
		    $foomaticstr .= "*End\n";
		}
		push(@optionblob, sprintf
		     ("\n*FoomaticRIPOption %s: enum %s %s %s\n",
		      $name, $optstyle, $spot, $order),
		     $foomaticstr);
	    }
	} elsif ($type eq 'bool') {
	    my $name = $arg->{'name'};
	    my $namef = $arg->{'name_false'};
	    my $defstr = ($default ? 'True' : 'False');
	    if (!defined($default)) { 
		$defstr = 'Unknown';
	    }
	    my $psstr = "";
	    my $psstrf = "";

	    push(@optionblob,
		 sprintf("\n*OpenUI *%s/%s: Boolean\n", $name, $com));

	    if ($arg->{'style'} eq 'G') {
		# Ghostscript argument
		$psstr = $cmd;
	    } else {
		# Option setting directive for Foomatic filter
		# 4 "%" because of the "sprintf" applied to it
		# In the end stay 2 "%" to have a PostScript comment
		my $header = sprintf
		    ("%%%% FoomaticRIPOptionSetting: %s", $name);
		$psstr = "$header=True";
		$psstrf = "$header=False";
		my $header = sprintf
		    ("*FoomaticRIPOptionSetting %s", $name);
		my $foomaticstr = ripdirective($header, $cmd) . "\n";
		# For non-PostScript options insert line with option
		# properties
		push(@optionblob, sprintf
		     ("*FoomaticRIPOption %s: bool %s %s\n",
		      $name, $optstyle, $spot).
		     $foomaticstr,
		     ($foomaticstr =~ /\n.*\n/s ? "*End\n" : ""));
	    }

	    push(@optionblob,
		 sprintf("*OrderDependency: %s AnySetup *%s\n", 
			 $order, $name),
		 sprintf("*Default%s: $defstr\n", $name),
		 sprintf("*%s True/%s: \"%s\"\n", $name, $name, $psstr),
		 ($psstr =~ /\n/s ? "*End\n" : ""),
		 sprintf("*%s False/%s: \"%s\"\n", $name, $namef, $psstrf),
		 ($psstrf =~ /\n/s ? "*End\n" : ""),
		 sprintf("*CloseUI: *%s\n", $name));
	    
	} elsif ($type eq 'int') {

	    # Real numerical options do not exist in the Adobe
	    # specification for PPD files. So we map the numerical
	    # options to enumerated options offering the minimum, the
	    # maximum, the default, and some values inbetween to the
	    # user.

	    my $min = $arg->{'min'};
	    my $max = $arg->{'max'};
	    my $second = $min + 1;
	    my $stepsize = 1;
	    if (($max - $min > 100) && ($name ne "Copies")) {
		# We don't want to have more than 100 values, but when the
		# difference between min and max is more than 100 we should
		# have at least 10 steps.
		my $mindesiredvalues = 10;
		my $maxdesiredvalues = 100;
		# Find the order of magnitude of the value range
		my $rangesize = $max - $min;
		my $log10 = log(10.0);
		my $rangeom = POSIX::floor(log($rangesize)/$log10);
		# Now find the step size
		my $trialstepsize = 10 ** $rangeom;
		my $numvalues = 0;
		while (($numvalues <= $mindesiredvalues) &&
		       ($trialstepsize > 2)) {
		    $trialstepsize /= 10;
		    $numvalues = $rangesize/$trialstepsize;
		}
		# Try to find a finer stepping
		$stepsize = $trialstepsize;
		$trialstepsize = $stepsize / 2;
		$numvalues = $rangesize/$trialstepsize;
		if ($numvalues <= $maxdesiredvalues) {
		    if ($stepsize > 20) { 
			$trialstepsize = $stepsize / 4;
			$numvalues = $rangesize/$trialstepsize;
		    }
		    if ($numvalues <= $maxdesiredvalues) {
			$trialstepsize = $stepsize / 5;
			$numvalues = $rangesize/$trialstepsize;
		    }
		    if ($numvalues <= $maxdesiredvalues) {
			$stepsize = $trialstepsize;
		    } else {
			$stepsize /= 2;
		    }
		}
		$numvalues = $rangesize/$stepsize;
		# We have the step size. Now we must find an appropriate
		# second value for the value list, so that it contains
		# the integer multiples of 10, 100, 1000, ...
		$second = $stepsize * POSIX::ceil($min / $stepsize);
		if ($second <= $min) {$second += $stepsize};
	    }
	    # Generate the choice list
	    my @choicelist;
	    push (@choicelist, $min);
	    if (($default < $second) && ($default > $min)) {
		push (@choicelist, $default);
	    }
	    my $item = $second;
	    while ($item < $max) {
		push (@choicelist, $item);
		if (($default < $item + $stepsize) && ($default > $item) &&
		    ($default < $max)) {
		    push (@choicelist, $default);
		}
		$item += $stepsize;
	    }
	    push (@choicelist, $max);

            # Add the option

	    # Skip zero or one choice arguments
	    if (1 < scalar(@choicelist)) {
		push(@optionblob,
		     sprintf("\n*OpenUI *%s/%s: PickOne\n", $name, $com));

		# Insert lines with the special properties of a
		# numerical option. Do this also for PostScript options
		# because numerical options are not supported by the PPD
		# file syntax. This way the info about this option being
		# a numerical one does not get lost

		push(@optionblob, sprintf
		     ("*FoomaticRIPOption %s: int %s %s\n",
		      $name, $optstyle, $spot));

		my $header = sprintf
		    ("*FoomaticRIPOptionPrototype %s",
		     $name);
		$foomaticstr = ripdirective($header, $cmd) . "\n";
		push(@optionblob, $foomaticstr);
		# Stuff to insert into command line/job is more than one
		# line? Let an "*End" line follow
		if ($foomaticstr =~ /\n.*\n/s) {
		    push(@optionblob, "*End\n");
		}

		push(@optionblob, sprintf
		     ("*FoomaticRIPOptionRange %s: %s %s\n",
		      $name, $arg->{'min'}, $arg->{'max'}));

		push(@optionblob,
		     sprintf("*OrderDependency: %s AnySetup *%s\n", 
			     $order, $name),
		     sprintf("*Default%s: %s\n", 
			     $name,
			     (defined($default) ? $default : 'Unknown')));
		if (!defined($default)) {
		    my $whr = sprintf("%s %s driver %s",
				      $dat->{'make'},
				      $dat->{'model'},
				      $dat->{'driver'});
		    warn "undefined default for $idx/$name on a $whr\n";
		}
	    
		my $v;
		for $v (@choicelist) {
		    my $psstr = "";
		    
		    if ($arg->{'style'} eq 'G') {
			# Ghostscript argument; offer up ps for insertion
			$psstr = sprintf($cmd, $v);
		    } else {
			# Option setting directive for Foomatic filter
			# 4 "%" because of the "sprintf" applied to it
			# In the end stay 2 "%" to have a PostScript comment
			$psstr = sprintf
			     ("%%%% FoomaticRIPOptionSetting: %s=%s",
			      $name, $v);
		    }
		    push(@optionblob,
			 sprintf("*%s %s/%s: \"%s\"\n", 
				 $name, $v, $v, $psstr));
		    # PostScript code is more than one line? Let an "*End"
		    # line follow
		    if ($psstr =~ /\n/s) {
			push(@optionblob, "*End\n");
		    }
		}
		
		push(@optionblob,
		     sprintf("*CloseUI: *%s\n", $name));
	    }
	    
	} elsif ($type eq 'float') {
	    
	    # Real numerical options do not exist in the Adobe
	    # specification for PPD files. So we map the numerical
	    # options to enumerated options offering the minimum, the
	    # maximum, the default, and some values inbetween to the
	    # user.

	    my $min = $arg->{'min'};
	    my $max = $arg->{'max'};
	    # We don't want to have more than 500 values or less than 50
	    # values.
	    my $mindesiredvalues = 10;
	    my $maxdesiredvalues = 100;
	    # Find the order of magnitude of the value range
	    my $rangesize = $max - $min;
	    my $log10 = log(10.0);
	    my $rangeom = POSIX::floor(log($rangesize)/$log10);
	    # Now find the step size
	    my $trialstepsize = 10 ** $rangeom;
	    my $stepom = $rangeom; # Order of magnitude of stepsize,
	                           # needed for determining necessary number
	                           # of digits
	    my $numvalues = 0;
	    while ($numvalues <= $mindesiredvalues) {
		$trialstepsize /= 10;
		$stepom -= 1;
		$numvalues = $rangesize/$trialstepsize;
	    }
	    # Try to find a finer stepping
	    $stepsize = $trialstepsize;
	    my $stepsizeorig = $stepsize;
	    $trialstepsize = $stepsizeorig / 2;
	    $numvalues = $rangesize/$trialstepsize;
	    if ($numvalues <= $maxdesiredvalues) {
		$stepsize = $trialstepsize;
		$trialstepsize = $stepsizeorig / 4;
		$numvalues = $rangesize/$trialstepsize;
		if ($numvalues <= $maxdesiredvalues) {
		    $stepsize = $trialstepsize;
		    $trialstepsize = $stepsizeorig / 5;
		    $numvalues = $rangesize/$trialstepsize;
		    if ($numvalues <= $maxdesiredvalues) {
			$stepsize = $trialstepsize;
		    }
		}
	    }
	    $numvalues = $rangesize/$stepsize;
	    if ($stepsize < $stepsizeorig * 0.9) {$stepom -= 1;}
	    # Determine number of digits after the decimal point for
	    # formatting the output values.
	    my $digits = 0;
	    if ($stepom < 0) {
		$digits = - $stepom;
	    }
	    # We have the step size. Now we must find an appropriate
	    # second value for the value list, so that it contains
	    # the integer multiples of 10, 100, 1000, ...
	    $second = $stepsize * POSIX::ceil($min / $stepsize);
	    if ($second <= $min) {$second += $stepsize};
	    # Generate the choice list
	    my @choicelist;
	    my $choicestr =  sprintf("%.${digits}f", $min);
	    push (@choicelist, $choicestr);
	    if (($default < $second) && ($default > $min)) {
		$choicestr =  sprintf("%.${digits}f", $default);
		# Prevent values from entering twice because of rounding
		# inacuracy
		if ($choicestr ne $choicelist[$#choicelist]) {
		    push (@choicelist, $choicestr);
		}
	    }
	    my $item = $second;
	    my $i = 0;
	    while ($item < $max) {
		$choicestr =  sprintf("%.${digits}f", $item);
		# Prevent values from entering twice because of rounding
		# inacuracy
		if ($choicestr ne $choicelist[$#choicelist]) {
		    push (@choicelist, $choicestr);
		}
		if (($default < $item + $stepsize) && ($default > $item) &&
		    ($default < $max)) {
		    $choicestr =  sprintf("%.${digits}f", $default);
		    # Prevent values from entering twice because of rounding
		    # inacuracy
		    if ($choicestr ne $choicelist[$#choicelist]) {
			push (@choicelist, $choicestr);
		    }
		}
		$i += 1;
		$item = $second + $i * $stepsize;
	    }
	    $choicestr =  sprintf("%.${digits}f", $max);
	    # Prevent values from entering twice because of rounding
	    # inacuracy
	    if ($choicestr ne $choicelist[$#choicelist]) {
		push (@choicelist, $choicestr);
	    }

            # Add the option

	    # Skip zero or one choice arguments
	    if (1 < scalar(@choicelist)) {
		push(@optionblob,
		     sprintf("\n*OpenUI *%s/%s: PickOne\n", $name, $com));

		# Insert lines with the special properties of a
		# numerical option. Do this also for PostScript options
		# because numerical options are not supported by the PPD
		# file syntax. This way the info about this option being
		# a numerical one does not get lost

		push(@optionblob, sprintf
		     ("*FoomaticRIPOption %s: float %s %s\n",
		      $name, $optstyle, $spot));

		my $header = sprintf
		    ("*FoomaticRIPOptionPrototype %s",
		     $name);
		$foomaticstr = ripdirective($header, $cmd) . "\n";
		push(@optionblob, $foomaticstr);
		# Stuff to insert into command line/job is more than one
		# line? Let an "*End" line follow
		if ($foomaticstr =~ /\n.*\n/s) {
		    push(@optionblob, "*End\n");
		}

		push(@optionblob, sprintf
		     ("*FoomaticRIPOptionRange %s: %s %s\n",
		      $name, $arg->{'min'}, $arg->{'max'}));

		push(@optionblob,
		     sprintf("*OrderDependency: %s AnySetup *%s\n", 
			     $order, $name),
		     sprintf("*Default%s: %s\n", 
			     $name,
			     (defined($default) ? 
			      sprintf("%.${digits}f", $default) : 'Unknown')));
		if (!defined($default)) {
		    my $whr = sprintf("%s %s driver %s",
				      $dat->{'make'},
				      $dat->{'model'},
				      $dat->{'driver'});
		    warn "undefined default for $idx/$name on a $whr\n";
		}

		my $v;
		for $v (@choicelist) {
		    my $psstr = "";
		    if ($arg->{'style'} eq 'G') {
			# Ghostscript argument; offer up ps for insertion
			$psstr = sprintf($cmd, $v);
		    } else {
			# Option setting directive for Foomatic filter
			# 4 "%" because of the "sprintf" applied to it
			# In the end stay 2 "%" to have a PostScript comment
			$psstr = sprintf
			     ("%%%% FoomaticRIPOptionSetting: %s=%s",
			      $name, $v);
		    }
		    push(@optionblob,
			 sprintf("*%s %s/%s: \"%s\"\n", 
				 $name, $v, $v, $psstr));
		    # PostScript code is more than one line? Let an "*End"
		    # line follow
		    if ($psstr =~ /\n/s) {
			push(@optionblob, "*End\n");
		    }
		}
		
		push(@optionblob,
		     sprintf("*CloseUI: *%s\n", $name));
	    }
        }
    }

    # Close the option groups which are still open
    for (my $i = $#groupstack; $i >= 0; $i--) {
	push(@optionblob,
	     sprintf("\n*Close%sGroup: %s\n",
		     ($i > 0 ? "Sub" : ""), $groupstack[$i])
	     );
	pop(@groupstack);
    }

    if (! $dat->{'args_byname'}{'PageSize'} ) {
	
	# This is a problem, since CUPS segfaults on PPD files without
	# a default PageSize set.  Indeed, the PPD spec requires a
	# PageSize clause.
	
	# GhostScript does not understand "/PageRegion[...]", therefore
	# we use "/PageSize[...]" in the "*PageRegion" option here, in
	# addition, for most modern PostScript interpreters "PageRegion"
	# is the same as "PageSize".

	push(@optionblob, <<EOFPGSZ);

*% This is fake. We have no information on how to
*% set the pagesize for this driver in the database. To
*% prevent PPD users from blowing up, we must provide a
*% default pagesize value.

*OpenUI *PageSize/Media Size: PickOne
*OrderDependency: 10 AnySetup *PageSize
*DefaultPageSize: Letter
*PageSize Letter/Letter: "<</PageSize[612 792]/ImagingBBox null>>setpagedevice"
*PageSize Legal/Legal: "<</PageSize[612 1008]/ImagingBBox null>>setpagedevice"
*PageSize A4/A4: "<</PageSize[595 842]/ImagingBBox null>>setpagedevice"
*CloseUI: *PageSize

*OpenUI *PageRegion: PickOne
*OrderDependency: 10 AnySetup *PageRegion
*DefaultPageRegion: Letter
*PageRegion Letter/Letter: "<</PageSize[612 792]/ImagingBBox null>>setpagedevice"
*PageRegion Legal/Legal: "<</PageSize[612 1008]/ImagingBBox null>>setpagedevice"
*PageRegion A4/A4: "<</PageSize[595 842]/ImagingBBox null>>setpagedevice"
*CloseUI: *PageRegion

*DefaultImageableArea: Letter
*ImageableArea Letter/Letter:	"0 0 612 792"
*ImageableArea Legal/Legal:	"0 0 612 1008"
*ImageableArea A4/A4:	"0 0 595 842"

*DefaultPaperDimension: Letter
*PaperDimension Letter/Letter:	"612 792"
*PaperDimension Legal/Legal:	"612 1008"
*PaperDimension A4/A4:	"595 842"

EOFPGSZ
    }

    my @others;

    my $headcomment =
"*% For information on using this, and to obtain the required backend
*% script, consult http://www.linuxprinting.org/foomatic2.9/
*%
*% This file is published under the GNU General Public License
*%
*% PPD-O-MATIC (2.9.x or newer) generated this PPD file. It is for use with 
*% all programs and environments which use PPD files for dealing with
*% printer capability information. The printer must be configured with the
*% \"foomatic-rip\" backend filter script of Foomatic 2.9.x or newer. This 
*% file and \"foomatic-rip\" work together to support PPD-controlled printer
*% driver option access with arbitrary free software printer drivers and
*% printing spoolers.
*%
*% To save this file on your disk, wait until the download has completed
*% (the animation of the browser logo must stop) and then use the
*% \"Save as...\" command in the \"File\" menu of your browser or in the 
*% pop-up manu when you click on this document with the right mouse button.
*% DO NOT cut and paste this file into an editor with your mouse. This can
*% introduce additional line breaks which lead to unexpected results.";

    my $postpipe = "";
    if ($dat->{'postpipe'}) {
	my $header = "*FoomaticRIPPostPipe";
	my $code = $dat->{'postpipe'};
	$postpipe = ripdirective($header, $code) . "\n";
	if ($postpipe =~ /\n.*\n/s) {
	    $postpipe .= "*End\n";
	}
    }
    my $blob = join('',@datablob);
    my $opts = join('',@optionblob);
    my $otherstuff = join('',@others);
    my $pcfilename;
    if (($dat->{'pcmodel'}) && ($dat->{'pcdriver'})) {
	$pcfilename = uc("$dat->{'pcmodel'}$dat->{'pcdriver'}");
    } else {
	my $driver = $dat->{'driver'};
	$driver =~ m!(^(.{1,8}))!;
	$pcfilename = uc($1);
    }
    my $model = $dat->{'model'};
    my $make = $dat->{'make'};
    my $pnpmodel;
    $pnpmodel = $dat->{'pnp_mdl'} or $pnpmodel = $dat->{'par_mdl'} or
	$pnpmodel = $dat->{'usb_mdl'} or $pnpmodel = $model;
    $pnpmodel = "($pnpmodel)" if $pnpmodel;
    my $pnpmake;
    $pnpmake = $dat->{'pnp_mfg'} or $pnpmake = $dat->{'par_mfg'} or
	$pnpmake = $dat->{'usb_mfg'} or $pnpmake = $make;
    my $filename = join('-',($dat->{'make'},
			     $dat->{'model'},
			     $dat->{'driver'}));;
    $filename =~ s![ /\(\)]!_!g;
    $filename =~ s![\+]!plus!g;
    $filename =~ s!__+!_!g;
    $filename =~ s!_$!!;
    $filename =~ s!_-!-!;
    $filename =~ s!^_!!;
    my $longname = "$filename.ppd";

    my $drivername = $dat->{'driver'};

    # Do we use the recommended driver?
    my $driverrecommended = "";
    if ($dat->{'driver'} eq $dat->{'recdriver'}) {
	$driverrecommended = " (recommended)";
    }
    
    # evil special case.
    $drivername = "stp-4.0" if $drivername eq 'stp';

    my $nickname = "$make $model, Foomatic + $drivername$driverrecommended";
    my $shortnickname = "$make $model, $drivername";

    my $tmpl = get_tmpl();
    $tmpl =~ s!\@\@POSTPIPE\@\@!$postpipe!g;
    $tmpl =~ s!\@\@HEADCOMMENT\@\@!$headcomment!g;
    $tmpl =~ s!\@\@SAVETHISAS\@\@!$longname!g;
    $tmpl =~ s!\@\@PCFILENAME\@\@!$pcfilename!g;
    $tmpl =~ s!\@\@PNPMAKE\@\@!$pnpmake!g;
    $tmpl =~ s!\@\@PNPMODEL\@\@!$pnpmodel!g;
    $tmpl =~ s!\@\@MODEL\@\@!$model!g;
    $tmpl =~ s!\@\@NICKNAME\@\@!$nickname!g;
    $tmpl =~ s!\@\@SHORTNICKNAME\@\@!$shortnickname!g;
    $tmpl =~ s!\@\@OTHERSTUFF\@\@!$otherstuff!g;
    $tmpl =~ s!\@\@OPTIONS\@\@!$opts!g;
    $tmpl =~ s!\@\@COMDATABLOB\@\@!$blob!g;
    $tmpl =~ s!\@\@PAPERDIMENSION\@\@!!g;
    
    return ($tmpl);
}


# Utility function; returns content of a URL
sub getpage {
    my ($this, $url, $dontdie) = @_;

    my $failed = 0;
    my $page = undef;
    # Try it first to retrieve the page with the "wget" shell command
    if (-x $sysdeps->{'wget'}) {
	if (open PAGE, "$sysdeps->{'wget'} $url -O - 2>/dev/null |") {
	    $page = join('', <PAGE>);
	    close PAGE;
	} else {
	    $failed = 1;
	}
    # Then try to retrieve the page with the "curl" shell command
    } elsif (-x $sysdeps->{'curl'}) {
	if (open PAGE, "$sysdeps->{'curl'} $url -o - 2>/dev/null |") {
	    $page = join('', <PAGE>);
	    close PAGE;
	} else {
	    $failed = 1;
	}
    } else {
	warn("WARNING: No tool for downloading web content found, please install either\n\"wget\" or \"curl\"! The result you got may be incorrect!\n");
    }

    if ((!$page) || ($failed)) {
	if ($dontdie) {
	    return undef;
	} else {
	    die ("http error: " . $request->status_line . "\n");
	}
    }

    return $page;
}

# Determine the margins as needed by "*ImageableArea"
sub getmarginsformarginrecord {
    my ($margins, $width, $height, $pagesize) = @_;
    if (!defined($margins)) {
	# No margins defined? Return zero margins
	return (0, $width, $height, 0);
    }
    # Defaults
    my $unit = 'pt';
    my $absolute = 0;
    my ($left, $right, $top, $bottom) = (0, $width, $height, 0);
    # Check the general margins and then the particular paper size
    for $i ('_general', $pagesize) {
	# Skip a section if it is not defined
	next if (!defined($margins->{$i}));
	# Determine the factor to calculate the margin in points (pt)
	$unit = (defined($margins->{$i}{'unit'}) ?
		 $margins->{$i}{'unit'} : $unit);
	my $unitfactor = 1.0; # Default unit is points
	if ($unit =~ /^p/i) {
	    $unitfactor = 1.0;
	} elsif ($unit =~ /^in/i) {
	    $unitfactor = 72.0;
	} elsif ($unit =~ /^cm$/i) {
	    $unitfactor = 72.0/2.54;
	} elsif ($unit =~ /^mm$/i) {
	    $unitfactor = 72.0/25.4;
	} elsif ($unit =~ /^dots(\d+)dpi$/i) {
	    $unitfactor = 72.0/$1;
	}
	# Convert the values to points
	($left, $right, $top, $bottom) =
	    ((defined($margins->{$i}{'left'}) ?
	      $margins->{$i}{'left'} * $unitfactor : $left),
	     (defined($margins->{$i}{'right'}) ?
	      $margins->{$i}{'right'} * $unitfactor : $right),
	     (defined($margins->{$i}{'top'}) ?
	      $margins->{$i}{'top'} * $unitfactor : $top),
	     (defined($margins->{$i}{'bottom'}) ?
	      $margins->{$i}{'bottom'} * $unitfactor : $bottom));
	# Determine the absolute values
	$absolute = (defined($margins->{$i}{'absolute'}) ?
		     $margins->{$i}{'absolute'} : $absolute);
	if (!$absolute){
	    if (defined($margins->{$i}{'right'})) {
		$right = $width - $right;
	    }
	    if (defined($margins->{$i}{'top'})) {
		$top = $height - $top;
	    }
	}
    }
    return ($left, $right, $top, $bottom);
}

sub getmargins {
    my ($dat, $width, $height, $pagesize) = @_;
    # Determine the unprintable margins
    # Zero margins when no margin info exists
    my ($left, $right, $top, $bottom) =
	(0, $width, $height, 0);
    # Margins from printer database entry
    my ($pleft, $pright, $ptop, $pbottom) =
	getmarginsformarginrecord($dat->{'printermargins'}, 
				  $width, $height, $pagesize);
    # Margins from driver database entry
    my ($dleft, $dright, $dtop, $dbottom) =
	getmarginsformarginrecord($dat->{'drivermargins'}, 
				  $width, $height, $pagesize);
    # Margins from printer/driver combo
    my ($cleft, $cright, $ctop, $cbottom) =
	getmarginsformarginrecord($dat->{'combomargins'}, 
				  $width, $height, $pagesize);
    # Left margin
    if ($pleft > $left) {$left = $pleft};
    if ($dleft > $left) {$left = $dleft};
    if ($cleft > $left) {$left = $cleft};
    # Right margin
    if ($pright < $right) {$right = $pright};
    if ($dright < $right) {$right = $dright};
    if ($cright < $right) {$right = $cright};
    # Top margin
    if ($ptop < $top) {$top = $ptop};
    if ($dtop < $top) {$top = $dtop};
    if ($ctop < $top) {$top = $ctop};
    # Bottom margin
    if ($pbottom > $bottom) {$bottom = $pbottom};
    if ($dbottom > $bottom) {$bottom = $dbottom};
    if ($cbottom > $bottom) {$bottom = $cbottom};
    # If we entered with $width == 0 and $height == 0, we mean
    # relative margins, so correct the signs
    if ($width == 0) {$right = -$right};
    if ($height == 0) {$top = -$top};
    # Clean up output
    $left =~ s/^\s*-0\s*$/0/;
    $right =~ s/^\s*-0\s*$/0/;
    $top =~ s/^\s*-0\s*$/0/;
    $bottom =~ s/^\s*-0\s*$/0/;
    # Return the results
    return ($left, $right, $top, $bottom);
}

# Generate a translation/longname from a shortname
sub longname {
    my $shortname = $_[0];
    # A space before every upper-case letter in the middle preceeded by
    # a lower-case one
    $shortname =~ s/([a-z])([A-Z])/$1 $2/g;
    # If there are three or more upper-case letters, assume the last as
    # the beginning of the next word, the others as an abbreviation
    $shortname =~ s/([A-Z][A-Z]+)([A-Z][a-z])/$1 $2/g;
    return $shortname;
}

# Prepare strings for being part of an HTML document by, converting
# "<" to "&lt;", ">" to "&gt;", "&" to "&amp;", "\"" to "&quot;",
# and "'" to  "&apos;"
sub htmlify {
    my $str = $_[0];
    $str =~ s!&!&amp;!g;
    $str =~ s/\</\&lt;/g;
    $str =~ s/\>/\&gt;/g;
    $str =~ s/\"/\&quot;/g;
    $str =~ s/\'/\&apos;/g;
    return $str;
}

# This splits RIP directives (PostScript comments which are
# foomatic-rip uses to build the RIP command line) into multiple lines
# of a fixed length, to avoid lines longer than 255 characters. The
# PPD specification does not allow such long lines.
sub ripdirective {
    my ($header, $content) = ($_[0], htmlify($_[1]));
    # If possible, make lines of this length
    my $maxlength = 72;
    # Header of continuation line
    my $continueheader = "";
    # Two subsequent ampersands are not possible in an htmlified string,
    # so we can use them at the line end to mark that the current line
    # continues on the next line. A newline without this is also a newline
    # in the decoded string
    my $continuelineend = "&&";
    # output string
    my $out;
    # The colon and the quote after the header must be on the line with
    # the header
    $header .= ": \"";
    # How much of the current line is left?
    my $freelength = $maxlength - length($header) -
	length($continuelineend);
    # Add the header
    if ($freelength < 0) {
	# header longer than $maxlength, don't break it
	$out = "$header$continuelineend\n$continueheader";
	$freelength = $maxlength - length($continueheader) -
	    length($continuelineend);
    } else {
	$out = "$header";
    }
    $content .= "\"";
    # Go through every line of the $content
    for $l (split ("\n", $content)) {
	while ($l) {
	    # Take off $maxlength portions until the string is used up
	    if (length($l) < $freelength) {
		$freelength = length($l);
	    }
	    my $line = substr($l, 0, $freelength, "");
	    # Add the portion 
	    $out .= $line;
	    # Finish the line
	    $freelength = $maxlength - length($continueheader) -
		length($continuelineend);
	    if ($l) {
		# Line continues in next line
		$out .= "$continuelineend\n$continueheader";
	    } else {
		# line ends
		$out .= "\n";
		last;
	    }
	}
    }
    # Remove trailing newline
    $out = substr($out, 0, -1);
    return $out;
}


# PPD boilerplate template

sub get_tmpl_paperdimension {
    return <<ENDPDTEMPL;
*% Generic PaperDimension; evidently there was no normal PageSize argument

*DefaultPaperDimension: Letter
*PaperDimension Letter:	"612 792"
*PaperDimension Legal:	"612 1008"
*PaperDimension A4:	"595 842"
ENDPDTEMPL
}

sub get_tmpl {
    return <<ENDTMPL;
*PPD-Adobe: "4.3"
\@\@POSTPIPE\@\@*%
\@\@HEADCOMMENT\@\@
*%
*% You may save this file as '\@\@SAVETHISAS\@\@'
*%
*%
*FormatVersion:	"4.3"
*FileVersion:	"1.1"
*LanguageVersion: English 
*LanguageEncoding: ISOLatin1
*PCFileName:	"\@\@PCFILENAME\@\@.PPD"
*Manufacturer:	"\@\@PNPMAKE\@\@"
*Product:	"\@\@PNPMODEL\@\@"
*cupsVersion:	1.0
*cupsManualCopies: True
*cupsModelNumber:  2
*cupsFilter:	"application/vnd.cups-postscript 0 foomatic-rip"
*%pprRIP:        foomatic-rip other
*ModelName:     "\@\@NICKNAME\@\@"
*ShortNickName: "\@\@SHORTNICKNAME\@\@"
*NickName:      "\@\@NICKNAME\@\@"
*PSVersion:	"(3010.000) 550"
*PSVersion:	"(3010.000) 651"
*PSVersion:	"(3010.000) 652"
*PSVersion:	"(3010.000) 653"
*PSVersion:	"(3010.000) 704"
*PSVersion:	"(3010.000) 705"
*PSVersion:	"(3010.000) 800"
*LanguageLevel:	"3"
*ColorDevice:	True
*DefaultColorSpace: RGB
*FileSystem:	False
*Throughput:	"1"
*LandscapeOrientation: Plus90
*TTRasterizer:	Type42
\@\@OTHERSTUFF\@\@
 
\@\@OPTIONS\@\@

*% Generic boilerplate PPD stuff as standard PostScript fonts and so on

\@\@PAPERDIMENSION\@\@

*DefaultFont: Courier
*Font AvantGarde-Book: Standard "(001.006S)" Standard ROM
*Font AvantGarde-BookOblique: Standard "(001.006S)" Standard ROM
*Font AvantGarde-Demi: Standard "(001.007S)" Standard ROM
*Font AvantGarde-DemiOblique: Standard "(001.007S)" Standard ROM
*Font Bookman-Demi: Standard "(001.004S)" Standard ROM
*Font Bookman-DemiItalic: Standard "(001.004S)" Standard ROM
*Font Bookman-Light: Standard "(001.004S)" Standard ROM
*Font Bookman-LightItalic: Standard "(001.004S)" Standard ROM
*Font Courier: Standard "(002.004S)" Standard ROM
*Font Courier-Bold: Standard "(002.004S)" Standard ROM
*Font Courier-BoldOblique: Standard "(002.004S)" Standard ROM
*Font Courier-Oblique: Standard "(002.004S)" Standard ROM
*Font Helvetica: Standard "(001.006S)" Standard ROM
*Font Helvetica-Bold: Standard "(001.007S)" Standard ROM
*Font Helvetica-BoldOblique: Standard "(001.007S)" Standard ROM
*Font Helvetica-Narrow: Standard "(001.006S)" Standard ROM
*Font Helvetica-Narrow-Bold: Standard "(001.007S)" Standard ROM
*Font Helvetica-Narrow-BoldOblique: Standard "(001.007S)" Standard ROM
*Font Helvetica-Narrow-Oblique: Standard "(001.006S)" Standard ROM
*Font Helvetica-Oblique: Standard "(001.006S)" Standard ROM
*Font NewCenturySchlbk-Bold: Standard "(001.009S)" Standard ROM
*Font NewCenturySchlbk-BoldItalic: Standard "(001.007S)" Standard ROM
*Font NewCenturySchlbk-Italic: Standard "(001.006S)" Standard ROM
*Font NewCenturySchlbk-Roman: Standard "(001.007S)" Standard ROM
*Font Palatino-Bold: Standard "(001.005S)" Standard ROM
*Font Palatino-BoldItalic: Standard "(001.005S)" Standard ROM
*Font Palatino-Italic: Standard "(001.005S)" Standard ROM
*Font Palatino-Roman: Standard "(001.005S)" Standard ROM
*Font Symbol: Special "(001.007S)" Special ROM
*Font Times-Bold: Standard "(001.007S)" Standard ROM
*Font Times-BoldItalic: Standard "(001.009S)" Standard ROM
*Font Times-Italic: Standard "(001.007S)" Standard ROM
*Font Times-Roman: Standard "(001.007S)" Standard ROM
*Font ZapfChancery-MediumItalic: Standard "(001.007S)" Standard ROM
*Font ZapfDingbats: Special "(001.004S)" Standard ROM

\@\@COMDATABLOB\@\@
ENDTMPL
}

# Determine the paper width and height in points from a given paper size
# name. Used for the "PaperDimension" and "ImageableArea" entries in PPD
# files.
#
# The paper sizes in the list are all sizes known to GhostScript, all
# of GIMP-Print, all sizes of HPIJS, and some others found in the data
# of printer drivers.

sub getpapersize {
    my $papersize = lc(join('', @_));

    my $sizetable = {
	'germanlegalfanfold' => '612 936',
	'letterwide'       => '647 957',
	'lettersmall'      => '612 792',
	'letter'           => '612 792',
	'legal'            => '612 1008',
	'postcard'         => '283 416',
	'tabloid'          => '792 1224',
	'ledger'           => '1224 792',
	'tabloidextra'     => '864 1296',
	'statement'        => '396 612',
	'manual'           => '396 612',
	'halfletter'       => '396 612',
	'executive'        => '522 756',
	'archa'            => '648 864',
	'archb'            => '864 1296',
	'archc'            => '1296 1728',
	'archd'            => '1728 2592',
	'arche'            => '2592 3456',
	'usaarch'          => '648 864',
	'usbarch'          => '864 1296',
	'uscarch'          => '1296 1728',
	'usdarch'          => '1728 2592',
	'usearch'          => '2592 3456',
	'a2.*invit.*'      => '315 414',
	'b6-c4'            => '354 918',
	'c7-6'             => '229 459',
	'supera3-b'        => '932 1369',
	'a3wide'           => '936 1368',
	'a4wide'           => '633 1008',
	'a4small'          => '595 842',
	'sra4'             => '637 907',
	'sra3'             => '907 1275',
	'sra2'             => '1275 1814',
	'sra1'             => '1814 2551',
	'sra0'             => '2551 3628',
	'ra4'              => '609 864',
	'ra3'              => '864 1218',
	'ra2'              => '1218 1729',
	'ra1'              => '1729 2437',
	'ra0'              => '2437 3458',
	'a10'              => '74 105',
	'a9'               => '105 148',
	'a8'               => '148 210',
	'a7'               => '210 297',
	'a6'               => '297 420',
	'a5'               => '420 595',
	'a4'               => '595 842',
	'a3'               => '842 1191',
	'a2'               => '1191 1684',
	'a1'               => '1684 2384',
	'a0'               => '2384 3370',
	'2a'               => '3370 4768',
	'4a'               => '4768 6749',
	'c10'              => '79 113',
	'c9'               => '113 161',
	'c8'               => '161 229',
	'c7'               => '229 323',
	'c6'               => '323 459',
	'c5'               => '459 649',
	'c4'               => '649 918',
	'c3'               => '918 1298',
	'c2'               => '1298 1836',
	'c1'               => '1836 2599',
	'c0'               => '2599 3676',
	'b10.*jis'         => '90 127',
	'b9.*jis'          => '127 180',
	'b8.*jis'          => '180 257',
	'b7.*jis'          => '257 362',
	'b6.*jis'          => '362 518',
	'b5.*jis'          => '518 727',
	'b4.*jis'          => '727 1029',
	'b3.*jis'          => '1029 1459',
	'b2.*jis'          => '1459 2063',
	'b1.*jis'          => '2063 2919',
	'b0.*jis'          => '2919 4127',
	'jis.*b10'         => '90 127',
	'jis.*b9'          => '127 180',
	'jis.*b8'          => '180 257',
	'jis.*b7'          => '257 362',
	'jis.*b6'          => '362 518',
	'jis.*b5'          => '518 727',
	'jis.*b4'          => '727 1029',
	'jis.*b3'          => '1029 1459',
	'jis.*b2'          => '1459 2063',
	'jis.*b1'          => '2063 2919',
	'jis.*b0'          => '2919 4127',
	'b10.*iso'         => '87 124',
	'b9.*iso'          => '124 175',
	'b8.*iso'          => '175 249',
	'b7.*iso'          => '249 354',
	'b6.*iso'          => '354 498',
	'b5.*iso'          => '498 708',
	'b4.*iso'          => '708 1000',
	'b3.*iso'          => '1000 1417',
	'b2.*iso'          => '1417 2004',
	'b1.*iso'          => '2004 2834',
	'b0.*iso'          => '2834 4008',
	'2b.*iso'          => '4008 5669',
	'4b.*iso'          => '5669 8016',
	'iso.*b10'         => '87 124',
	'iso.*b9'          => '124 175',
	'iso.*b8'          => '175 249',
	'iso.*b7'          => '249 354',
	'iso.*b6'          => '354 498',
	'iso.*b5'          => '498 708',
	'iso.*b4'          => '708 1000',
	'iso.*b3'          => '1000 1417',
	'iso.*b2'          => '1417 2004',
	'iso.*b1'          => '2004 2834',
	'iso.*b0'          => '2834 4008',
	'iso.*2b'          => '4008 5669',
	'iso.*4b'          => '5669 8016',
	'b10envelope'      => '87 124',
	'b9envelope'       => '124 175',
	'b8envelope'       => '175 249',
	'b7envelope'       => '249 354',
	'b6envelope'       => '354 498',
	'b5envelope'       => '498 708',
	'b4envelope'       => '708 1000',
	'b3envelope'       => '1000 1417',
	'b2envelope'       => '1417 2004',
	'b1envelope'       => '2004 2834',
	'b0envelope'       => '2834 4008',
	'b10'              => '87 124',
	'b9'               => '124 175',
	'b8'               => '175 249',
	'b7'               => '249 354',
	'b6'               => '354 498',
	'b5'               => '498 708',
	'b4'               => '708 1000',
	'b3'               => '1000 1417',
	'b2'               => '1417 2004',
	'b1'               => '2004 2834',
	'b0'               => '2834 4008',
	'monarch'          => '279 540',
	'dl'               => '311 623',
	'com10'            => '297 684',
	'com.*10'          => '297 684',
	'hagaki'           => '283 420',
	'oufuku'           => '420 567',
	'kaku'             => '680 941',
	'long.*3'          => '340 666',
	'long.*4'          => '255 581',
	'foolscap'         => '576 936',
	'flsa'             => '612 936',
	'flse'             => '648 936',
	'photo100x150'     => '283 425',
	'photo200x300'     => '567 850',
	'photofullbleed'   => '298 440',
	'photo4x6'         => '288 432',
	'photo'            => '288 432',
	'wide'             => '977 792',
	'card148'          => '419 297',
	'envelope132x220'  => '374 623',
	'envelope61/2'     => '468 260',
	'supera'           => '644 1008',
	'superb'           => '936 1368',
	'fanfold5'         => '612 792',
	'fanfold4'         => '612 864',
	'fanfold3'         => '684 792',
	'fanfold2'         => '864 612',
	'fanfold1'         => '1044 792',
	'fanfold'          => '1071 792',
	'panoramic'        => '595 1683',
	'plotter.*size.*a' => '612 792',
	'plotter.*size.*b' => '792 1124',
	'plotter.*size.*c' => '1124 1584',
	'plotter.*size.*d' => '1584 2448',
	'plotter.*size.*e' => '2448 3168',
	'plotter.*size.*f' => '3168 4896',
	'archlarge'        => '162 540',
	'standardaddr'     => '81 252',
	'largeaddr'        => '101 252',
	'suspensionfile'   => '36 144',
	'videospine'       => '54 423',
	'badge'            => '153 288',
	'archsmall'        => '101 540',
	'videotop'         => '130 223',
	'diskette'         => '153 198',
	'76\.2mmroll'      => '216 0',
	'69\.5mmroll'      => '197 0',
	'roll'             => '612 0',
	'custom'           => '0 0'
	};

    # Remove prefixes which sometimes could appear
    $papersize =~ s/form_//;

    # Check whether the paper size name is in the list above
    for $item (keys(%{$sizetable})) {
	if ($papersize =~ /$item/) {
	    return $sizetable->{$item};
	}
    }

    # Check if we have a "<Width>x<Height>" format, assume the numbers are
    # given in inches
    if ($papersize =~ /(\d+)x(\d+)/) {
	my $w = $1 * 72;
	my $h = $2 * 72;
	return sprintf("%d %d", $w, $h);
    }

    # Check if we have a "w<Width>h<Height>" format, assume the numbers are
    # given in points
    if ($papersize =~ /w(\d+)h(\d+)/) {
	return "$1 $2";
    }

    # Check if we have a "w<Width>" format, assume roll paper with the given
    # width in points
    if ($papersize =~ /w(\d+)/) {
	return "$1 0";
    }

    # This paper size is absolutely unknown, issue a warning
    warn "WARNING: Unknown paper size: $papersize!";
    return "0 0";
}

# Get documentation for the printer/driver pair to print out. For
# "Execution Details" section of driver web pages on linuxprinting.org

sub getexecdocs {

    my ($this) = $_[0];

    my $dat = $this->{'dat'};

    my @docs;
    
    # Construct the proper command line.
    my $commandline = htmlify($dat->{'cmd'});

    if ($commandline eq "") {return ();}

    my @letters = qw/A B C D E F G H I J K L M Z/;
    my $spot;
    
    for $spot (@letters) {
	
	if($commandline =~ m!\%$spot!) {

	    my $arg;
	  argument:
	    for $arg (@{$dat->{'args'}}) {
#	    for $arg (sort { $a->{'order'} <=> $b->{'order'} } 
#		      @{$dat->{'args'}}) {
		
		# Only do arguments that go in this spot
		next argument if ($arg->{'spot'} ne $spot);
		# PJL arguments are not inserted at a spot in the command
		# line
		next argument if ($arg->{'style'} eq 'J');
		# Composite options are not interesting here
		next argument if ($arg->{'style'} eq 'X');
		
		my $name = htmlify($arg->{'name'});
		my $varname = htmlify($arg->{'varname'});
		my $cmd = htmlify($arg->{'proto'});
		my $comment = htmlify($arg->{'comment'});
		my $placeholder = "</TT><I>&lt;$name&gt;</I><TT>";
		my $default = htmlify($arg->{'default'});
		my $type = $arg->{'type'};
		my $cmdvar = "";
		my $gsarg1 = "";
		my $gsarg2 = "";
		if ($arg->{'style'} eq 'G') {
		    $gsarg1 = ' -c "';
		    $gsarg2 = '"';
		    $cmd =~ s/\"/\\\"/g;
		}
		#my $leftbr = ($arg->{'required'} ? "" : "[");
		#my $rightbr = ($arg->{'required'} ? "" : "]");
		my $leftbr = "";
		my $rightbr = "";
	
		if ($type eq 'bool') {
		    $cmdvar = "$leftbr$gsarg1$cmd$gsarg2$rightbr";
		} elsif ($type eq 'int' or $type eq 'float') {
		    $cmdvar = sprintf("$leftbr$gsarg1$cmd$gsarg2$rightbr",$placeholder);
		} elsif ($type eq 'enum') {
		    my $val;
		    if ($val=valbyname($arg,$default)) {
			$cmdvar = sprintf("$leftbr$gsarg1$cmd$gsarg2$rightbr",
					  $placeholder);
		    }
		}
		
		# Insert the processed argument in the commandline
		# just before every occurance of the spot marker.
		$cmdvar =~ s!^\[\ !\ \[!;
		$commandline =~ s!\%$spot!$cmdvar\%$spot!g;
	    }
	    
	    # Remove the letter markers from the commandline
	    $commandline =~ s!\%$spot!!g;
	    
	}
	
    }

    $dat->{'excommandline'} = $commandline;

    push(@docs, "<B>Command Line</B><P>");
    push(@docs, "<BLOCKQUOTE><TT>$commandline</TT></BLOCKQUOTE><P>");

    my ($arg, @doctmp);
    my @pjlcommands = ();
  argt:
    for $arg (@{$dat->{'args'}}) {
#    for $arg (sort { $a->{'order'} <=> $b->{'order'} } 
#	      @{$dat->{'args'}}) {

	# Composite options are not interesting here
	next argt if ($arg->{'style'} eq 'X');

	# Make sure that the longname/translation exists
	if (!$arg->{'comment'}) {
	    $arg->{'comment'} = longname($arg->{'name'});
	}

	my $name = htmlify($arg->{'name'});
	my $cmd = htmlify($arg->{'proto'});
	my $comment = htmlify($arg->{'comment'});
	my $placeholder = "</TT><I>&lt;$name&gt;</I><TT>";
	if ($arg->{'style'} eq 'J') {
	    $cmd = "\@PJL $cmd";
	    push (@pjlcommands, sprintf($cmd, $placeholder));
	}

	my $default = htmlify($arg->{'default'});
	my $type = $arg->{'type'};
	
	my $required = ($arg->{'required'} ? " required" : "n optional");
	my $pjl = ($arg->{'style'} eq 'J' ? "PJL " : "");

	if ($type eq 'bool') {
	    my $name_false = htmlify($arg->{'name_false'});
	    push(@doctmp,
		 "<DL><DT><I>$name</I></DT>",
		 "<DD>A$required boolean ${pjl}argument meaning $name if present or $name_false if not.<BR>",
		 "$comment<BR>",
		 "Prototype: <TT>$cmd</TT><BR>",
		 "Default: ", $default ? "True" : "False",
		 "</DD></DL><P>"
		 );

	} elsif ($type eq 'int' or $type eq 'float') {
	    my $max = (defined($arg->{'max'}) ? $arg->{'max'} : "none");
	    my $min = (defined($arg->{'min'}) ? $arg->{'min'} : "none");
	    push(@doctmp,
		 "<DL><DT><I>$name</I></DT>",
		 "<DD>A$required $type ${pjl}argument.<BR>",
		 "$comment<BR>",
		 "Prototype: <TT>", sprintf($cmd, $placeholder),
		 "</TT><BR>",
		 "Default: <TT>$default</TT><BR>",
		 "Range: <TT>$min &lt;= $placeholder &lt;= $max</TT>",
		 "</DD></DL><P>"
		 );

	} elsif ($type eq 'enum') {
	    my ($val, $defstr);
	    my (@choicelist) = ();

	    for $val (@{$arg->{'vals'}}) {

		# Make sure that the longname/translation exists
		if (!$val->{'comment'}) {
		    $val->{'comment'} = longname($val->{'value'});
		}

		my ($value, $comment, $driverval) = 
		    (htmlify($val->{'value'}),
		     htmlify($val->{'comment'}),
		     htmlify($val->{'driverval'}));

		if (defined($driverval)) {
		    if ($driverval eq "") {
			push(@choicelist,
			     "<LI>$value: $comment (<TT>$placeholder</TT> is left blank)</LI>");
		    } else {
			my $widthheight = "";
			if (($name eq "PageSize") && ($value eq "Custom")) {
			    my $width = "</TT><I>&lt;Width&gt;</I><TT>";
			    my $height = "</TT><I>&lt;Height&gt;</I><TT>";
			    $driverval =~ s/\%0/$width/ or
                            $driverval =~ s/(\W)0(\W)/$1$width$2/ or
                            $driverval =~ s/^0(\W)/$width$1/m or
                            $driverval =~ s/(\W)0$/$1$width/m or
                            $driverval =~ s/^0$/$width/m;
                            $driverval =~ s/\%1/$height/ or
                            $driverval =~ s/(\W)0(\W)/$1$height$2/ or
                            $driverval =~ s/^0(\W)/$height$1/m or
                            $driverval =~ s/(\W)0$/$1$height/m or
                            $driverval =~ s/^0$/$height/m;
			    $widthheight = ", <I>&lt;Width&gt;</I> and <I>&lt;Height&gt;</I> are the page dimensions in points, 1/72 inches";
			}
			push(@choicelist,
			     "<LI>$value: $comment (<TT>$placeholder</TT> is '<TT>$driverval</TT>'$widthheight)</LI>");
		    }
		} else {
		    push(@choicelist,
			 "<LI>$value: $comment (<TT>$placeholder</TT> is '<TT>$value</TT>')</LI>");
		}
	    }

	    push(@doctmp,
		 "<DL><DT><I>$name</I></DT>",
		 "<DD>A$required enumerated choice ${pjl}argument.<BR>",
		 "$comment<BR>",
		 "Prototype: <TT>", sprintf($cmd, $placeholder),
		 "</TT><BR>",
		 "Default: $default",
		 "<UL>", 
		 join("", @choicelist), 
		 "</UL></DD></DL><P>"
		 );

	}
    }

    # Instructions for PJL commands
    if (($#pjlcommands > -1) && (defined($dat->{'pjl'}))) {
    #if (($#pjlcommands > -1)) {
	my @pjltmp;
	push(@pjltmp,
	     "PJL arguments are not put into the command line, they must be put into a PJL header which is prepended to the actual job data which is generated by the command line shown above and sent to the printer. After the job data one can reset the printer via PJL. So a complete job looks as follows:<BLOCKQUOTE>",
	     "<I>&lt;ESC&gt;</I>",
	     # The "JOB" PJL command is not supported by all printers
	     "<TT>%-12345X\@PJL</TT><BR>");
	     #"<TT>%-12345X\@PJL JOB NAME=\"</TT>",
	     #"<I>&lt;A job name&gt;</I>",
	     #"<TT>\"</TT><BR>");
	for $command (@pjlcommands) {
	    push(@pjltmp,
		 "<TT>$command</TT><BR>");
	}
	push(@pjltmp,
	     "<I>&lt;The job data&gt;</I><BR>",
	     "<I>&lt;ESC&gt;</I>",
	     # The "JOB" PJL command is not supported by all printers
	     "<TT>%-12345X\@PJL RESET</TT></BLOCKQUOTE><P>",
	     #"<TT>%-12345X\@PJL EOJ</TT></BLOCKQUOTE><P>",
	     "<I>&lt;ESC&gt;</I>",
	     ": This is the ",
	     "<I>ESC</I>",
	     " character, ASCII code 27.<P>",
	     #"<I>&lt;A job name&gt;</I>",
	     #": The job name can be chosen arbitrarily, some printers show it on their front panel displays.<P>",
	     "It is not required to give the PJL arguments, you can leave out some of them or you can even send only the job data without PJL header and PJL end-of-job mark.<P>");
	push(@docs, "<B>PJL</B><P>");
	push(@docs, @pjltmp);
    } elsif ((defined($dat->{'drivernopjl'})) && 
	     ($dat->{'drivernopjl'} == 1) && 
	     (defined($dat->{'pjl'}))) {
	my @pjltmp;
	push(@pjltmp,
	     "This driver produces a PJL header with PJL commands internally, so commands in a PJL header sent to the printer before the output of this driver would be ignored. Therefore there are no PJL options available when using this driver.<P>");
	push(@docs, "<B>PJL</B><P>");
	push(@docs, @pjltmp);
    }

    push(@docs, "<B>Options</B><P>");

    push(@docs, @doctmp);

    return @docs;
   
}

# Get a shorter summary documentation thing.
sub get_summarydocs {
    my ($this) = $_[0];

    my $dat = $this->{'dat'};

    my @docs;

    for $arg (@{$dat->{'args'}}) {

	# Make sure that the longname/translation exists
	if (!$arg->{'comment'}) {
	    $arg->{'comment'} = longname($arg->{'name'});
	}

	my ($name,
	    $required,
	    $type,
	    $comment,
	    $spot,
	    $default) = ($arg->{'name'},
			 $arg->{'required'},
			 $arg->{'type'},
			 $arg->{'comment'},
			 $arg->{'spot'},
			 $arg->{'default'});
	
	my $reqstr = ($required ? " required" : "n optional");
	push(@docs,
	     "Option `$name':\n  A$reqstr $type argument.\n  $comment\n");

	push(@docs,
	     "  This option corresponds to a PJL command.\n") 
	    if ($spot eq 'Y');
	
	if ($type eq 'bool') {
	    if (defined($default)) {
		my $defstr = ($default ? "True" : "False");
		push(@docs, "  Default: $defstr\n");
	    }
	    push(@docs, "  Example (true): `$name'\n");
	    push(@docs, "  Example (false): `no$name'\n");
	} elsif ($type eq 'enum') {
	    push(@docs, "  Possible choices:\n");
	    my $exarg;
	    for (@{$arg->{'vals'}}) {

		# Make sure that the longname/translation exists
		if (!$_->{'comment'}) {
		    $_->{'comment'} = longname($_->{'value'});
		}

		my ($choice, $comment) = ($_->{'value'}, $_->{'comment'});
		push(@docs, "   * $choice: $comment\n");
		$exarg=$choice;
	    }
	    if (defined($default)) {
		push(@docs, "  Default: $default\n");
	    }
	    push(@docs, "  Example: `$name=$exarg'\n");
	} elsif ($type eq 'int' or $type eq 'float') {
	    my ($max, $min) = ($arg->{'max'}, $arg->{'min'});
	    my $exarg;
	    if (defined($max)) {
		push(@docs, "  Range: $min <= x <= $max\n");
		$exarg=$max;
	    }
	    if (defined($default)) {
		push(@docs, "  Default: $default\n");
		$exarg=$default;
	    }
	    if (!$exarg) { $exarg=0; }
	    push(@docs, "  Example: `$name=$exarg'\n");
	}

	push(@docs, "\n");
    }

    return @docs;

}

# About as obsolete as the other docs functions.  Why on earth are
# there three, anyway?!
sub getdocs {
    my ($this) = $_[0];

    my $dat = $this->{'dat'};

    my @docs;

    for $arg (@{$dat->{'args'}}) {

	# Make sure that the longname/translation exists
	if (!$arg->{'comment'}) {
	    $arg->{'comment'} = longname($arg->{'name'});
	}

	my ($name,
	    $required,
	    $type,
	    $comment,
	    $spot,
	    $default) = ($arg->{'name'},
			 $arg->{'required'},
			 $arg->{'type'},
			 $arg->{'comment'},
			 $arg->{'spot'},
			 $arg->{'default'});
	
	my $reqstr = ($required ? " required" : "n optional");
	push(@docs,
	     "Option `$name':\n  A$reqstr $type argument.\n  $comment\n");

	push(@docs,
	     "  This option corresponds to a PJL command.\n") 
	    if ($spot eq 'Y');
	
	if ($type eq 'bool') {
	    if (defined($default)) {
		my $defstr = ($default ? "True" : "False");
		push(@docs, "  Default: $defstr\n");
	    }
	    push(@docs, "  Example (true): `$name'\n");
	    push(@docs, "  Example (false): `no$name'\n");
	} elsif ($type eq 'enum') {
	    push(@docs, "  Possible choices:\n");
	    my $exarg;
	    for (@{$arg->{'vals'}}) {

		# Make sure that the longname/translation exists
		if (!$_->{'comment'}) {
		    $_->{'comment'} = longname($_->{'value'});
		}

		my ($choice, $comment) = ($_->{'value'}, $_->{'comment'});
		push(@docs, "   * $choice: $comment\n");
		$exarg=$choice;
	    }
	    if (defined($default)) {
		push(@docs, "  Default: $default\n");
	    }
	    push(@docs, "  Example: `$name=$exarg'\n");
	} elsif ($type eq 'int' or $type eq 'float') {
	    my ($max, $min) = ($arg->{'max'}, $arg->{'min'});
	    my $exarg;
	    if (defined($max)) {
		push(@docs, "  Range: $min <= x <= $max\n");
		$exarg=$max;
	    }
	    if (defined($default)) {
		push(@docs, "  Default: $default\n");
		$exarg=$default;
	    }
	    if (!$exarg) { $exarg=0; }
	    push(@docs, "  Example: `$name=$exarg'\n");
	}

	push(@docs, "\n");
    }

    return @docs;

}

# Find a choice value hash by name.
# Operates on old dat structure...
sub valbyname {
    my ($arg,$name) = @_;

    my $val;
    for $val (@{$arg->{'vals'}}) {
	return $val if (lc($name) eq lc($val->{'value'}));
    }

    return undef;
}

# replace numbers with fixed 6-digit number for ease of sorting
# ie: sort { normalizename($a) cmp normalizename($b) } @foo;
sub normalizename {
    my $n = $_[0];

    $n =~ s/[\d\.]+/sprintf("%013.6f", $&)/eg;
    return $n;
}


# Load an XML object from the library
# You specify the relative file path (to .../db/), less the .xml on the end.
sub _get_object_xml {
    my ($this, $file, $quiet) = @_;

    open XML, "$libdir/db/$file.xml"
	or do { warn "Cannot open file $libdir/db/$file.xml\n"
		    if !$quiet;
		return undef; };
    my $xml = join('', (<XML>));
    close XML;

    return $xml;
}

# Write an XML object from the library
# You specify the relative file path (to .../db/), less the .xml on the end.
sub _set_object_xml {
    my ($this, $file, $stuff, $cache) = @_;

    my $dir = "$libdir/db";
    my $xfile = "$dir/$file.xml";
    umask 0002;
    open XML, ">$xfile.$$"
	or do { warn "Cannot write file $xfile.$$\n";
		return undef; };
    print XML $stuff;
    close XML;
    rename "$xfile.$$", $xfile
	or die "Cannot rename $xfile.$$ to $xfile\n";

    return 1;
}

# Get a list of XML filenames from a library directory.  These could then be
# read with _get_object_xml.
sub _get_xml_filelist {
    my ($this, $dir) = @_;

    if (!defined($this->{"names-$dir"})) {
	opendir DRV, "$libdir/db/$dir"
	    or die 'Cannot find source db for $dir\n';
	my $driverfile;
	while($driverfile = readdir(DRV)) {
	    next if ($driverfile !~ m!^(.+)\.xml$!);
	    push(@{$this->{"names-$dir"}}, $1);
	}
	closedir(DRV);
    }

    return @{$this->{"names-$dir"}};
}


# Return a Perl structure in eval-able ascii format
sub getascii {
    my ($this) = $_[0];
    if (! $this->{'dat'}) {
	$this->getdat();
    }
    
    local $Data::Dumper::Purity=1;
    local $Data::Dumper::Indent=1;

    # Encase data for inclusion in PPD file
    return Dumper($this->{'dat'});
}

# Return list of printer makes
sub get_makes {
    my ($this) = @_;

    my @makes;
    my %seenmakes;
    my $p;
    for $p (@{$this->get_overview()}) {
	my $make = $p->{'make'};
	push (@makes, $make) 
	    if ! $seenmakes{$make}++;
    }
	
    return @makes;
	
}

# get a list of model names from a make
sub get_models_by_make {
    my ($this, $wantmake) = @_;

    my $over = $this->get_overview();

    my @models;
    my $p;
    for $p (@{$over}) {
	push (@models, $p->{'model'}) 
	    if ($wantmake eq $p->{'make'});
    }

    return @models;
}

# get a printer id from a make/model
sub get_printer_from_make_model {
    my ($this, $wantmake, $wantmodel) = @_;

    my $over = $this->get_overview();
    my $p;
    for $p (@{$over}) {
	return $p->{'id'} if ($p->{'make'} eq $wantmake
			      and $p->{'model'} eq $wantmodel);
    }

    return undef;
}

sub get_javascript2 {

    my ($this) = @_;

    my @swit;
    my $mak;
    my $else = "";
    for $mak ($this->get_makes()) {
	push (@swit,
	      " $else if (make == \"$mak\") {\n");

	my $ct = 0;
	my $mod;
	for $mod (sort {normalizename($a) cmp normalizename($b) } 
		  $this->get_models_by_make($mak)) {
	    
	    my $p;
	    $p = $this->get_printer_from_make_model($mak, $mod);
	    if (defined($p)) {
		push (@swit,
		      "      o[i++]=new Option(\"$mod\", \"$p\");\n");
		$ct++;
	    }
	}

	if (!$ct) {
	    push(@swit,
		 "      o[i++]=new Option(\"No Printers\", \"0\");\n");
	}

	push (@swit,
	      "    }");
	$else = "else";
    }

    my $switch = join('',@swit);

    my $javascript = '
       function reflectMake(makeselector, modelselector) {
	 //
	 // This function is called when makeselector changes
	 // by an onchange thingy on the makeselector.
	 //

	 // Get the value of the OPTION that just changed
	 selected_value=makeselector.options[makeselector.selectedIndex].value;
	 // Get the text of the OPTION that just changed
	 make=makeselector.options[makeselector.selectedIndex].text;

	 o = new Array;
	 i=0;

     ' . $switch . '    if (i==0) {
	   alert("Error: that dropdown should do something, but it doesnt");
	 } else {
	   modelselector.length=o.length;
	   for (i=0; i < o.length; i++) {
	     modelselector.options[i]=o[i];
	   }
	   modelselector.options[0].selected=true;
	 }

       }
     ';

    return $javascript;
}

################################3
#################################


# Modify comments text to contain only what it should:
#
# <a>, <p>, <br> (<br> -> <p>)
#
sub comment_filter {
    my ($text) = @_;

    my $fake = ("INSERTFIXEDTHINGHERE" . sprintf("%06x", rand(1000000)));
    my %replacements;
    my $num = 1;

    # extract all the A href tags
    my $replace = "ANCHOR$fake$num";
    while ($text =~ 
	   s!(<\s*a\s+href\s*=\s*['"]([^'"]+)['"]\s*>)!$replace!i) {
	$replacements{$replace} = $1;
	$num++;
	$replace = "ANCHOR$fake$num";
    }

    # extract all the A tail tags
    my $replace = "ANCHORTAIL$fake$num";
    while ($text =~ 
	   s!(<\s*/\s*a\s*>)!$replace!i) {
	$replacements{$replace} = $1;
	$num++;
	$replace = "ANCHOR$fake$num";
    }

    # extract all the P tags
    $replace = "PARA$fake$num";
    while ($text =~ 
	   s!(<\s*p\s*>)!$replace!i) {

	$replacements{$replace} = $1;
	$num++;
	$replace = "PARA$fake$num";
    }

    # extract all the BR tags
    $replace = "PARA$fake$num";
    while ($text =~ 
	   s!(<\s*br\s*>)!$replace!i) {

	$replacements{$replace} = $1;
	$num++;
	$replace = "PARA$fake$num";
    }

    # Now it's just clean text; remove all tags and &foo;s
    $text =~ s!<[^>]+>! !g;
    $text =~ s!&amp;!&!g;
    $text =~ s!&lt;!<!g;
    $text =~ s!&gt;!>!g;
    $text =~ s!&[^;]+?;! !g;

    # Now rewrite into our teeny-html subset
    $text =~ s!&!&amp;!g;
    $text =~ s!<!&lt;!g;
    $text =~ s!>!&gt;!g;

    # And reinsert the few things we wanted to preserve
    for (keys(%replacements)) {
	my ($k, $r) = ($_, $replacements{$_});
	$text =~ s!$k!$r!;
    }

#    print STDERR "$text";

    return $text;
}

1;
