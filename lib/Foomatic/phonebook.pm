package Foomatic::phonebook;

use strict;
use warnings;

sub new {
	my ($class, $version) = @_;
	my $self = {};
	bless $self, $class;
	
	if(defined($version)) {
		$self->{'version'} = $version;
	} else {
		$self->{'version'} = 0;
	}
	
	return $self;
}


### Phonebook ###
# A phonebook is an array that describes an XML node.
# It contains xpaths, destination hash keys
# it also groups nodes. A node group are nodes which
# can be processed by the same function.
#
# ----- group enumerated type table ------
# 0 = no group, this group is not processed at all
# 
# General groups
# 1 = simple text node
# 2 = boolean, if present set to true
# 3 = boolean, if present set to false
# 4 = id, removes leading directory name
# 5 = Margins, present in printer and driver xmls
# 6 = Human readable text, legacy and proper multi-lingual support ($version > 1)
# 7 = text node with whitespace stripping
#
# Printer Specific groups
# 12 = "drivers", plural, specific group
# 13 = languages specific group
# 14 = printer types
#
# Driver Specific groups
# 11 = "type" specific group
# 12 = "printers" specific group
# 13 = "support contacts" group
# 14 = Packages group
# 15 = License link, human readable
#
# Option Specific groups
# 11 = constraints
# 12 = enum_vals
# 13 = style group, hardcoded replacements

my $printerPhonebook = [
	["/printer/ppdentry[1]", 'ppdentry', 0],
	["/printer/make[1]", "make", 1],
	["/printer/model[1]", "model", 1],
	["/printer/url[1]", "url", 1],
	["/printer/contrib_url[1]", "contriburl", 1],
	["/printer/functionality[1]", "functionality", 1],
	["/printer/driver[1]", "driver", 1],
	["/printer/mechanism/resolution/dpi/x[1]", "maxxres",1],
	["/printer/mechanism/resolution/dpi/y[1]", "maxyres",1],
	["/printer/autodetect/general/description[1]", "general_des",1],
	["/printer/autodetect/general/manufacturer[1]", "general_mfg",1],
	["/printer/autodetect/general/model[1]", "general_mdl",1],
	["/printer/autodetect/general/ieee1284[1]", "general_ieee",1],
	["/printer/autodetect/general/commandset[1]", "general_cmd",1],
	["/printer/autodetect/snmp/description[1]", "snmp_des",1],
	["/printer/autodetect/snmp/manufacturer[1]", "snmp_mfg",1],
	["/printer/autodetect/snmp/model[1]", "snmp_mdl",1],
	["/printer/autodetect/snmp/ieee1284[1]", "snmp_ieee",1],
	["/printer/autodetect/snmp/commandset[1]", "snmp_cmd",1],
	["/printer/autodetect/usb/description[1]", "usb_des",1],
	["/printer/autodetect/usb/manufacturer[1]", "usb_mfg",1],
	["/printer/autodetect/usb/model[1]", "usb_mdl",1],
	["/printer/autodetect/usb/ieee1284[1]", "usb_ieee",1],
	["/printer/autodetect/usb/commandset[1]", "usb_cmd",1],
	["/printer/autodetect/parallel/description[1]", "par_des",1],
	["/printer/autodetect/parallel/manufacturer[1]", "par_mfg",1],
	["/printer/autodetect/parallel/model[1]", "par_mdl",1],
	["/printer/autodetect/parallel/ieee1284[1]", "par_ieee",1],
	["/printer/autodetect/parallel/commandset[1]", "par_cmd",1],
	["/printer/lang/text/charset[1]", "ascii",1],
	["/printer/mechanism/color", "color",2],
	["/printer/lang/pjl", "pjl",2],
	["/printer/\@id", "id",4],
	["/printer/mechanism/margins", "margins", 5],
	["/printer/comments", "comment", 6],
	["/printer/drivers", "drivers", 12],
	["/printer/lang", "languages", 13],
	["/printer/mechanism/inkjet", "type",14],
	["/printer/mechanism/transfer", "type",14],
	["/printer/mechanism/laser", "type",14],
	["/printer/mechanism/sublimation", "type",14],
	["/printer/mechanism/dotmatrix", "type",14],
	["/printer/mechanism/impact", "type",14],
	["/printer/mechanism/led", "type",14],
];
sub printer {
	my ($this) = @_;
	my $phonebook = $printerPhonebook;
	if($this->{'version'} > 0) {
		$phonebook = [
			@{$phonebook}, 
			["/printer/pcmodel", "pcmodel", 1],#group 6 manages this
		];
	}
	if($this->{'version'} > 1) {
		$phonebook = [
			@{$phonebook}, 
			["/printer/comments", "comments", 0],#group 6 manages this
		];
	}
	return $phonebook;
}

my $driverPhonebook = [
	["/driver/name[1]", "name", 1],
	["/driver/url[1]", "url", 1],
	["/driver/execution/prototype[1]", "cmd", 1],
	["/driver/execution/prototype_pdf[1]", "cmd_cmd", 1],
	["/driver/execution/ppdentry[1]", "ppdentry", 1],
	["/driver/license[1]", "license", 1],
	["/driver/supplier[1]", "supplier", 1],
	["/driver/functionality/maxresx[1]", "drvmaxresx", 1],
	["/driver/functionality/maxresy[1]", "drvmaxresy", 1],
	["/driver/functionality/graphics[1]", "graphics", 1],
	["/driver/functionality/lineart[1]", "lineart", 1],
	["/driver/functionality/text[1]", "text", 1],
	["/driver/functionality/photo[1]", "photo", 1],
	["/driver/functionality/speed[1]", "speed", 1],
	["/driver/obsolete/\@replace", "obsolete",1],
	["/driver/patents", "patents", 2],
	["/driver/functionality/color", "drvcolor", 2],
	["/driver/thirdpartysupplied", "manufacturersupplied", 3],
	["/driver/manufacturersupplied", "manufacturersupplied", 2],
	["/driver/manufacturersupplied", "manufacturersupplied", 1],#special order
	["/driver/nonfreesoftware", "free", 3],
	["/driver/functionality/monochrome", "drvcolor", 3],
	["/driver/execution/margins", "margins", 5],
	["/driver/comments", "comment", 6],
	["/driver/licensetext", "origlicensetext", 6],
	["/driver/licensetext", "licensetext", 6],
	["/driver/shortdescription/en[1]", "shortdescription", 7],
	["/driver/execution/ghostscript", "type",11],
	["/driver/execution/uniprint", "type",11],
	["/driver/execution/filter", "type",11],
	["/driver/execution/cups", "type",11],
	["/driver/execution/ijs", "type",11],
	["/driver/execution/postscript", "type",11],
	["/driver/printers", "printers", 12],
	["/driver/supportcontacts", "supportcontacts", 13],
	["/driver/packages", "packages", 14],
	["/driver/licensetext", "licenselink", 15],
];
sub driver {
	my ($this) = @_;
	my $phonebook = $driverPhonebook;
	if($this->{'version'} > 0) {
		$phonebook = [
			@{$phonebook}, 
			["/driver/pcdriver", "pcdriver", 1],
			["/driver/execution/nopjl", "nopjl", 2],
			["/driver/execution/nopageaccounting", "nopageaccounting", 2],
		];
	}
	if($this->{'version'} > 1) {
		$phonebook = [
			@{$phonebook}, 
			["/driver/comments", "comments", 0], #Multiligual fields
			["/driver/licensetext", "origlicensetexts", 0],
			["/driver/licensetext", "licensetexts", 0],
		];
	}
	return $phonebook;
}

my $optionPhonebook = [
	["/option/\@id", "idx", 1],
	["/option/arg_execution/arg_group[1]", "group", 1],
	["/option/arg_execution/arg_order[1]", "order", 1],
	["/option/arg_execution/arg_spot[1]", "spot", 1],
	["/option/arg_execution/arg_proto[1]", "proto", 1],
	["/option/arg_execution/arg_section[1]", "section", 1],
	["/option/arg_allowedchars[1]", "allowedchars", 1],
	["/option/arg_maxlength", "maxlength", 1],
	["/option/arg_max[1]", "max", 1],
	["/option/arg_min[1]", "min", 1],
	["/option/\@type", "type", 4],
	["/option/arg_longname", "comment", 6],
	["/option/arg_shortname", "name", 7],
	["/option/arg_shortname_false", "name_false", 7],
	["/option/constraints", "constraints", 11],
	["/option/enum_vals", "vals", 12],
	["/option/arg_execution/arg_substitution", "style", 13],
	["/option/arg_execution/arg_postscript", "style", 13],
	["/option/arg_execution/arg_pjl", "style", 13],
	["/option/arg_execution/arg_forced_composite", "substyle", 13],
];
sub option {
	my ($this) = @_;
	my $phonebook = $optionPhonebook;
	if($this->{'version'} > 1) {
		$phonebook = [
			@{$phonebook}, 
			["/option/arg_longname", "comments", 0], #Multiligual fields
		];
	}
	return $phonebook;
}

my $comboPhonebook = 
{ 'printer' => [#printer keys
	['model', ''],
	['make', ''],
	['pcmodel', ''],
	['id', ''],
	['pjl', ''],
	['color', ''],
	['drivers', ''],
	['driver', 'recdriver'],
	['margins', 'printermargins'],
	['general_mfg', ''],
	['general_des', ''],
	['general_mdl', ''],
	['general_ieee', ''],
	['general_cmd', ''],
	['general_mfg', 'pnp_mfg'],
	['general_des', 'pnp_des'],
	['general_mdl', 'pnp_mdl'],
	['general_ieee', 'pnp_ieee'],
	['general_cmd', 'pnp_cmd'],
	['snmp_mfg', ''],
	['snmp_des', ''],
	['snmp_mdl', ''],
	['snmp_ieee', ''],
	['snmp_cmd', ''],
	['usb_mfg', ''],
	['usb_des', ''],
	['usb_mdl', ''],
	['usb_ieee', ''],
	['usb_cmd', ''],
	['par_mfg', ''],
	['par_des', ''],
	['par_mdl', ''],
	['par_ieee', ''],
	['par_cmd', ''],
], 'driver' => [ #driver keys
	['url', ''],
	['name', 'driver'],
	['type', ''],
	['pcdriver', ''],
	['comment', ''],
	['cmd', ''],
	['color', ''],
	['packages', ''],
	['license', ''],
	['supplier', ''],
	['supportcontacts', ''],
	['patents', ''],
	['graphics', ''],
	['text', ''],
	['lineart', ''],
	['photo', ''],
	['speed', ''],
	['drvcolor', ''],
	['drvmaxresx', ''],
	['drvmaxresy', ''],
	['shortdescription', ''],
	['manufacturersupplied', ''],
	['licensetext', ''],
	['origlicensetext', ''],
	['nopjl', 'drivernopjl'],
	['ppdentry', 'driverppdentry'],
	['margins', 'drivermargins'],
	['combomargins', ''],
	['nopageaccounting', 'drivernopageaccounting'],
]};
sub combo {
	return $comboPhonebook;
}

1;
