package Foomatic::filters::xml::to;

use strict;
use warnings;
use Data::Dumper;

use DBI;
use Foomatic::filters::phonebook;
use Foomatic::util;


sub new( $ $ $ ) {
    my $class = shift;
    my $this = {};
    return bless($this, $class);
}

sub generateDriver {
    my ($this, $data) = @_;
    return $this->generator('d', $data);
}

sub generateOption {
    #Not actually functional
    my ($this, $data) = @_;
    return $this->generator('o', $data);
}

sub generatePrinter {
    my ($this, $data) = @_;
    return $this->generator('p', $data);
}

sub generateCombo {
    my ($this, $data) = @_;
    return $this->generator('c', $data);
}


sub generator {
    my ($this, $mode, $dat) = @_;

    my $xml = "";

    $xml .= "<foomatic>\n" if !$mode || ($mode =~ /^c/i); 

    if (!$mode || ($mode =~ /^[cp]/i)) { 
	$xml .=
	    "<printer id=\"printer/" . $dat->{'id'} . "\">\n" .
	    "  <make>" . $dat->{'make'} . "</make>\n" .
	    "  <model>" . $dat->{'model'} . "</model>\n" .
	    "  <mechanism>\n" .
	    ($dat->{'type'} ? "    <" . $dat->{'type'} . " />\n" : ()) .
	    ($dat->{'color'} ? "    <color />\n" : ()) .
	    ($dat->{'maxxres'} || $dat->{'maxyres'} ?
	     "    <resolution>\n" .
	     "      <dpi>\n" .
	     ($dat->{'maxxres'} ?
	      "        <x>" . $dat->{'maxxres'} . "</x>\n" : ()) .
	     ($dat->{'maxyres'} ?
	      "        <y>" . $dat->{'maxyres'} . "</y>\n" : ()) .
	     "      </dpi>\n" .
	     "    </resolution>\n" : ()) .
	     "  </mechanism>\n";
	if (defined($dat->{'languages'}) ||
	    defined($dat->{'pjl'}) ||
	    defined($dat->{'ascii'})) {
	    $xml .= "  <lang>\n";
	    if (defined($dat->{'languages'})) {
		for  my $lang (@{$dat->{'languages'}}) {
		    $xml .= "    <" . $lang->{'name'};
		    if ($lang->{'level'}) {
			$xml .= " level=\"" . $lang->{'level'} . "\"";
		    }
		    $xml .= " />\n";
		}
	    }
	    if (defined($dat->{'pjl'})) {
		$xml .= "    <pjl />\n";
	    }
	    if (defined($dat->{'ascii'})) {
		$xml .= "    <text>\n";
		$xml .= "      <charset>us-ascii</charset>\n";
		$xml .= "    </text>\n";
	    }
	    $xml .= "  </lang>\n";
	}
	if (defined($dat->{'general_ieee'}) ||
	    defined($dat->{'general_mfg'}) ||
	    defined($dat->{'general_mdl'}) ||
	    defined($dat->{'general_des'}) ||
	    defined($dat->{'general_cmd'})) {
	    $xml .= "  <autodetect>\n";
	    $xml .= "    <general>\n";
	    $xml .= "      <ieee1284>" . $dat->{'general_ieee'} .
		"</ieee1284>\n" if defined($dat->{'general_ieee'});
	    $xml .= "      <manufacturer>" . $dat->{'general_mfg'} .
		"</manufacturer>\n" if defined($dat->{'general_mfg'});
	    $xml .= "      <model>" . $dat->{'general_mdl'} .
		"</model>\n" if defined($dat->{'general_mdl'});
	    $xml .= "      <description>" . $dat->{'general_des'} .
		"</description>\n" if defined($dat->{'general_des'});
	    $xml .= "      <commandset>" . $dat->{'general_cmd'} .
		"</commandset>\n" if defined($dat->{'general_cmd'});
	    $xml .= "    </general>\n";
	    $xml .= "  </autodetect>\n";
	}
	$xml .= "  <functionality>" . $dat->{'functionality'} .
	    "</functionality>\n" if defined($dat->{'functionality'});
	$xml .= "  <driver>" . $dat->{'driver'} .
	    "</driver>\n" if defined($dat->{'driver'});
	if (defined($dat->{'drivers'})) {
	    $xml .= "  <drivers>\n";
	    for  my $drv (sort {lc($a->{'id'}) cmp lc($b->{'id'})}
			  @{$dat->{'drivers'}}) {
		$xml .= "    <driver>\n";
		$xml .= "      <id>" . $drv->{'id'} . "</id>\n"
		    if defined($drv->{'id'});
		$xml .= "      <ppd>" . $drv->{'ppd'} . "</ppd>\n"
		    if defined($drv->{'ppd'});
		$xml .= "    </driver>\n";
	    }
	    $xml .= "  </drivers>\n";
	}
	$xml .= "  <unverified />\n" if $dat->{'unverified'};
	$xml .=
	    "  <comments>\n" .
	    "    <en>\n";
	$xml .= htmlify($dat->{'comment'}) . "\n" if $dat->{'comment'};
	$xml .=
	    "    </en>\n" .
	    "  </comments>\n" .
	    "</printer>\n";
    }

    if (!$mode || ($mode =~ /^[cd]/i)) { 
	$xml .=
	    "<driver id=\"driver/" . $dat->{'driver'} . "\">\n" .
	    "  <name>" . $dat->{'driver'} . "</name>\n" .
	    "  <execution>\n" .
	    "    <filter />\n" .
	    "    <prototype>" . $dat->{'cmd'} . "</prototype>\n" .
	    $dat->{'cmd_pdf'} ? 
		"    <prototype_pdf>" . $dat->{'cmd_pdf'} . "</prototype_pdf>\n" :
		"" .
	    "  </execution>\n" .
	    "</driver>\n\n";
    }

    if (!$mode || ($mode =~ /^c/i)) { 
	$xml .= "<options>\n";

	foreach (@{$dat->{'args'}}) {
	    my $type = $_->{'type'};
	    my $optname = $_->{'name'};
	    $xml .= "  <option type=\"$type\" " .
		"id=\"opt/" . $dat->{'driver'} . "-" . $optname . "\">\n";
	    $xml .=
		"    <arg_longname>\n" .
		"      <en>" . $_->{'comment'} . "</en>\n" .
		"    </arg_longname>\n" .
		"    <arg_shortname>\n" .
		"      <en>" . $_->{'name'} . "</en>\n" .
		"    </arg_shortname>\n" .
		"    <arg_execution>\n";
	    $xml .= "      <arg_group>" . $_->{'group'} . "</arg_group>\n"
		if $_->{'group'};
	    $xml .= "      <arg_order>" . $_->{'order'} . "</arg_order>\n"
		if $_->{'order'};
	    $xml .= "      <arg_spot>" . $_->{'spot'} . "</arg_spot>\n"
		if $_->{'spot'};
	    $xml .= "      <arg_proto>" . $_->{'proto'} . "</arg_proto>\n"
		if $_->{'proto'};
	    $xml .= "    </arg_execution>\n";
	    
	    if ($type eq 'enum') {
		$xml .= "    <enum_vals>\n";
		my $vals_byname = $_->{'vals_byname'};
		foreach (keys(%{$vals_byname})) {
		    my $val = $vals_byname->{$_};
		    $xml .=
			"      <enum_val id=\"ev/" . $dat->{'driver'} . "-" .
			$optname . "-" . $_ . "\">\n";
		    $xml .=
			"        <ev_longname>\n" .
			"          <en>" . $val->{'comment'} . "</en>\n" .
			"        </ev_longname>\n" .
			"        <ev_shortname>\n" .
			"          <en>$_</en>\n" .
			"        </ev_shortname>\n";

		    $xml .=
			"        <ev_driverval>" .
			$val->{'driverval'} .
			"</ev_driverval>\n" if $val->{'driverval'};

		    $xml .= "      </enum_val>\n";
		}
		$xml .= "    </enum_vals>\n";
	    }

	    $xml .= "  </option>\n";
	}

	$xml .= "</options>\n";
	$xml .= "</foomatic>\n";
    }
    return $xml;
}


1;

