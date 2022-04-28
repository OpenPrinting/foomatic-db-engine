package Foomatic::filters::phonebook;

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
	["/printer/ppdentry[1]", 'ppdentry', 0, 'printer', '', 0],
	["/printer/make[1]", "make", 1, 'printer', '', 0],
	["/printer/model[1]", "model", 1, 'printer', '', 0],
	["/printer/url[1]", "url", 1, 'printer', '', 0],
	["/printer/contrib_url[1]", "contriburl", 1, 'printer', 'contrib_url', 0],
	["/printer/functionality[1]", "functionality", 1, 'printer', '', 0],
	["/printer/driver[1]", "driver", 1, 'printer', 'default_driver', 0],
	["/printer/mechanism/resolution/dpi/x[1]", "maxxres",1, 'printer', 'res_x', 0],
	["/printer/mechanism/resolution/dpi/y[1]", "maxyres",1, 'printer', 'res_y', 0],
	["/printer/autodetect/general/description[1]", "general_des",1, 'printer', 'general_description', 0],
	["/printer/autodetect/general/manufacturer[1]", "general_mfg",1, 'printer', 'general_manufacturer', 0],
	["/printer/autodetect/general/model[1]", "general_mdl",1, 'printer', 'general_model', 0],
	["/printer/autodetect/general/ieee1284[1]", "general_ieee",1, 'printer', 'general_ieee1284', 0],
	["/printer/autodetect/general/commandset[1]", "general_cmd",1, 'printer', 'general_commandset', 0],
	["/printer/autodetect/snmp/description[1]", "snmp_des",1, 'printer', 'snmp_description', 0],
	["/printer/autodetect/snmp/manufacturer[1]", "snmp_mfg",1, 'printer', 'snmp_manufacturer', 0],
	["/printer/autodetect/snmp/model[1]", "snmp_mdl",1, 'printer', 'snmp_model', 0],
	["/printer/autodetect/snmp/ieee1284[1]", "snmp_ieee",1, 'printer', 'snmp_ieee1284', 0],
	["/printer/autodetect/snmp/commandset[1]", "snmp_cmd",1, 'printer', 'snmp_commandset', 0],
	["/printer/autodetect/usb/description[1]", "usb_des",1, 'printer', 'usb_description', 0],
	["/printer/autodetect/usb/manufacturer[1]", "usb_mfg",1, 'printer', 'usb_manufacturer', 0],
	["/printer/autodetect/usb/model[1]", "usb_mdl",1, 'printer', 'usb_model', 0],
	["/printer/autodetect/usb/ieee1284[1]", "usb_ieee",1, 'printer', 'usb_ieee1284', 0],
	["/printer/autodetect/usb/commandset[1]", "usb_cmd",1, 'printer', 'usb_commandset', 0],
	["/printer/autodetect/parallel/description[1]", "par_des",1, 'printer', 'parallel_description', 0],
	["/printer/autodetect/parallel/manufacturer[1]", "par_mfg",1, 'printer', 'parallel_manufacturer', 0],
	["/printer/autodetect/parallel/model[1]", "par_mdl",1, 'printer', 'parallel_model', 0],
	["/printer/autodetect/parallel/ieee1284[1]", "par_ieee",1, 'printer', 'parallel_ieee1284', 0],
	["/printer/autodetect/parallel/commandset[1]", "par_cmd",1, 'printer', 'parallel_commandset', 0],
	["/printer/lang/text/charset[1]", "ascii",1, 'printer', 'text', 0],
	["/printer/mechanism/color", "color",2, 'printer', '', 0],
	["/printer/lang/pjl", "pjl",2, 'printer', '', 0],
	["/printer/\@id", "id",4, 'printer', 'id', 0],
	["/printer/mechanism/margins", "margins", 5, 'margin', '', 0],
	["/printer/comments", "comment", 6, 'printer', 'comments', 0],
	["/printer/drivers", "drivers", 12, 'driver_printer_assoc', '', 0],
	["/printer/lang", "languages", 13, 'printer', '', 0],
	["/printer/mechanism/inkjet", "type",14, 'printer', 'mechanism', 0],
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
			["/printer/pcmodel", "pcmodel", 1, 'printer', '', 0],
		];
	}
	if($this->{'version'} > 1) {
		$phonebook = [
			@{$phonebook}, 
			["/printer/comments", "comments", 0, 'printer_translation', '',7],#group 6 manages this
		];
	}
	return $phonebook;
}

my $driverPhonebook = [
	["/driver/name[1]", "name", 1, 'driver', '', 0],
	["/driver/name[1]", "name", 0, 'driver', 'id', 0],
	["/driver/url[1]", "url", 1, 'driver', '', 0],
	["/driver/execution/prototype[1]", "cmd", 1, 'driver', 'prototype', 0],
	["/driver/execution/prototype_pdf[1]", "cmd_cmd", 1, 'driver', 'pdf_prototype', 0],
	["/driver/execution/ppdentry[1]", "ppdentry", 1, 'driver', '', 0],
	["/driver/license[1]", "license", 1, 'driver', '', 0],
	["/driver/supplier[1]", "supplier", 1, 'driver', '', 0],
	["/driver/functionality/maxresx[1]", "drvmaxresx", 1, 'driver', 'max_res_x', 0],
	["/driver/functionality/maxresy[1]", "drvmaxresy", 1, 'driver', 'max_res_y', 0],
	["/driver/functionality/graphics[1]", "graphics", 1, 'driver', '', 0],
	["/driver/functionality/lineart[1]", "lineart", 1, 'driver', '', 0],
	["/driver/functionality/text[1]", "text", 1, 'driver', '', 0],
	["/driver/functionality/photo[1]", "photo", 1, 'driver', '', 0],
	["/driver/functionality/speed[1]", "speed", 1, 'driver', '', 0],
	["/driver/obsolete/\@replace", "obsolete",1, 'driver', '', 0],
	["/driver/patents", "patents", 2, 'driver', '', 0],
	["/driver/functionality/color", "drvcolor", 2, 'driver', 'color', 0],
	["/driver/thirdpartysupplied", "manufacturersupplied", 3, 'driver', '', 4],
	["/driver/manufacturersupplied", "manufacturersupplied", 2, 'driver', 'thirdpartysupplied', 3],
	["/driver/manufacturersupplied", "manufacturersupplied", 1],#special order
	["/driver/nonfreesoftware", "free", 3, 'driver', 'nonfreesoftware', 3],
	["/driver/functionality/monochrome", "drvcolor", 3, 'driver', 'color', 0],
	["/driver/execution/margins", "margins", 5, 'margin', '', 0],
	["/driver/comments", "comment", 6, 'driver', 'comments', 0],
	["/driver/licensetext", "origlicensetext", 6],
	["/driver/licensetext", "licensetext", 6, 'driver', '', 0],
	["/driver/shortdescription/en[1]", "shortdescription", 7, 'driver', '', 0],
	["/driver/execution/ghostscript", "type",11, 'driver', 'execution', 0],
	["/driver/execution/uniprint", "type",11],
	["/driver/execution/filter", "type",11],
	["/driver/execution/cups", "type",11],
	["/driver/execution/ijs", "type",11],
	["/driver/execution/postscript", "type",11],
	["/driver/printers", "printers", 12, 'driver_printer_assoc', '', 0],
	["/driver/supportcontacts", "supportcontacts", 13, 'driver_support_contact', '', 0],
	["/driver/packages", "packages", 14, 'driver_package', '', 0],
	["/driver/licensetext", "licenselink", 15, 'driver', '', 0],
];
sub driver {
	my ($this) = @_;
	my $phonebook = $driverPhonebook;
	if($this->{'version'} > 0) {
		$phonebook = [
			@{$phonebook}, 
			["/driver/pcdriver", "pcdriver", 1, 'driver', '', 0],
			["/driver/execution/nopjl", "nopjl", 2, 'driver', 'no_pjl', 0],
			["/driver/execution/nopageaccounting", "nopageaccounting", 2, 'driver', 'no_pageaccounting', 0],
		];
	}
	if($this->{'version'} > 1) {
		$phonebook = [
			@{$phonebook}, 
			["/driver/comments", "comments", 0, 'driver_translation', 'comments', 7], #Multiligual fields
			["/driver/licensetext", "origlicensetexts", 0],
			["/driver/licensetext", "licensetexts", 0],
		];
	}
	return $phonebook;
}

my $optionPhonebook = [
	["/option/\@id", "idx", 1, 'options', 'id', 4],
	["/option/arg_execution/arg_group[1]", "group", 1, 'options', 'option_group', 0],
	["/option/arg_execution/arg_order[1]", "order", 1, 'options', 'option_order', 0],
	["/option/arg_execution/arg_spot[1]", "spot", 1, 'options', 'option_spot', 0],
	["/option/arg_execution/arg_proto[1]", "proto", 1, 'options', 'prototype', 0],
	["/option/arg_execution/arg_section[1]", "section", 1, 'options', 'option_section', 0],
	["/option/arg_allowedchars[1]", "allowedchars", 1, 'options', 'allowed_chars', 0],
	["/option/arg_maxlength", "maxlength", 1, 'options', '', 0],
	["/option/arg_max[1]", "max", 1, 'options', 'max_value', 0],
	["/option/arg_min[1]", "min", 1, 'options', 'min_value', 0],
	["/option/\@type", "type", 4, 'options', 'option_type', 0],
	["/option/arg_longname", "comment", 6, 'options', 'longname', 0],
	["/option/arg_shortname", "name", 7, 'options', 'shortname', 0],
	["/option/arg_shortname_false", "name_false", 7, 'options', 'shortname_false', 0],
	["/option/constraints", "constraints", 11, 'option_constraint', '', 0],
	["/option/enum_vals", "vals", 12, 'option_choice', '', 0],
	["/option/arg_execution/arg_substitution", "style", 13, 'options', 'execution', 0],
	["/option/arg_execution/arg_postscript", "style", 13],
	["/option/arg_execution/arg_pjl", "style", 13],
	["/option/arg_execution/arg_forced_composite", "substyle", 13, 'options', 'execution', 0],
];
sub option {
	my ($this) = @_;
	my $phonebook = $optionPhonebook;
	if($this->{'version'} > 1) {
		$phonebook = [
			@{$phonebook}, 
			["/option/arg_longname", "comments", 0, 'options_translation', '',7], #Multiligual fields
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
	['obsolete', ''],
	['cmd', ''],
	['cmd_cmd', 'cmd_pdf'],
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
	['comboppdentry', ''],
	['nopageaccounting', 'drivernopageaccounting'],
]};
sub combo {
	return $comboPhonebook;
}

#SQL SCHEMAS

my $schemaPhonebook = 
{ 'printer' => 'CREATE TABLE "printer" (
  "id" varchar(50) NOT NULL,
  "make" varchar(40) NOT NULL,
  "model" varchar(40) NOT NULL,
  "pcmodel" varchar(6) DEFAULT NULL,
  "functionality" varchar(255) NOT NULL,
  "mechanism" varchar(255),
  "color" integer NOT NULL DEFAULT \'0\',
  "res_x" integer NOT NULL DEFAULT \'0\',
  "res_y" integer NOT NULL DEFAULT \'0\',
  "url" tinytext,
  "unverified" integer NOT NULL DEFAULT \'0\',
  "postscript" integer NOT NULL DEFAULT \'0\',
  "postscript_level" varchar(8) DEFAULT NULL,
  "pdf" integer NOT NULL DEFAULT \'0\',
  "pdf_level" varchar(8) DEFAULT NULL,
  "pcl" integer NOT NULL DEFAULT \'0\',
  "pcl_level" varchar(8) DEFAULT NULL,
  "escp" integer NOT NULL DEFAULT \'0\',
  "escp_level" varchar(8) DEFAULT NULL,
  "escp2" integer NOT NULL DEFAULT \'0\',
  "escp2_level" varchar(8) DEFAULT NULL,
  "hpgl2" integer NOT NULL DEFAULT \'0\',
  "hpgl2_level" varchar(8) DEFAULT NULL,
  "tiff" integer NOT NULL DEFAULT \'0\',
  "tiff_level" varchar(8) DEFAULT NULL,
  "lips" integer NOT NULL DEFAULT \'0\',
  "lips_level" varchar(8) DEFAULT NULL,
  "proprietary" integer NOT NULL DEFAULT \'0\',
  "pjl" integer NOT NULL DEFAULT \'0\',
  "text" varchar(10) DEFAULT NULL,
  "general_ieee1284" text,
  "general_manufacturer" varchar(40) DEFAULT NULL,
  "general_model" varchar(40) DEFAULT NULL,
  "general_commandset" varchar(30) DEFAULT NULL,
  "general_description" varchar(1024) DEFAULT NULL,
  "parallel_ieee1284" text,
  "parallel_manufacturer" varchar(40) DEFAULT NULL,
  "parallel_model" varchar(40) DEFAULT NULL,
  "parallel_commandset" varchar(30) DEFAULT NULL,
  "parallel_description" varchar(1024) DEFAULT NULL,
  "usb_ieee1284" text,
  "usb_manufacturer" varchar(40) DEFAULT NULL,
  "usb_model" varchar(40) DEFAULT NULL,
  "usb_commandset" varchar(30) DEFAULT NULL,
  "usb_description" varchar(1024) DEFAULT NULL,
  "snmp_ieee1284" text,
  "snmp_manufacturer" varchar(40) DEFAULT NULL,
  "snmp_model" varchar(40) DEFAULT NULL,
  "snmp_commandset" varchar(30) DEFAULT NULL,
  "snmp_description" varchar(1024) DEFAULT NULL,
  "default_driver" varchar(50) DEFAULT NULL,
  "ppdentry" text,
  "contrib_url" tinytext,
  "comments" text
)', 
'printer_translation' => 'CREATE TABLE "printer_translation" (
  "id" varchar(50) NOT NULL,
  "lang" varchar(8) NOT NULL DEFAULT \'\',
  "comments" text,
  CONSTRAINT "printer_translation_ibfk_1" FOREIGN KEY ("id") REFERENCES "printer" ("id") ON DELETE CASCADE
)', 
'options' => 'CREATE TABLE "options" (
  "id" varchar(50) NOT NULL,
  "option_type" varchar(255) NOT NULL,
  "shortname" varchar(50) NOT NULL,
  "longname" varchar(50) DEFAULT NULL,
  "execution" varchar(255) NOT NULL DEFAULT \'substitution\',
  "required" integer DEFAULT \'0\',
  "prototype" text,
  "option_spot" varchar(10) DEFAULT NULL,
  "option_order" varchar(10) DEFAULT NULL,
  "option_section" varchar(50) DEFAULT NULL,
  "option_group" tinytext,
  "comments" text,
  "max_value" integer DEFAULT NULL,
  "min_value" integer DEFAULT NULL,
  "shortname_false" varchar(50) DEFAULT NULL,
  "maxlength" integer DEFAULT NULL,
  "allowed_chars" text,
  "allowed_regexp" text
)', 
'options_translation' => 'CREATE TABLE "options_translation" (
  "id" varchar(50) NOT NULL,
  "lang" varchar(8) NOT NULL DEFAULT \'\',
  "longname" varchar(50) DEFAULT NULL,
  "comments" text,
  CONSTRAINT "options_translation_ibfk_1" FOREIGN KEY ("id") REFERENCES "options" ("id") ON DELETE CASCADE
)',
'options_constraint' => 'CREATE TABLE "option_constraint" (
  "option_id" varchar(50) NOT NULL,
  "choice_id" varchar(50) NOT NULL DEFAULT \'\',
  "sense" varchar(255) NOT NULL,
  "driver" varchar(50) NOT NULL DEFAULT \'\',
  "printer" varchar(50) NOT NULL DEFAULT \'\',
  "defval" varchar(1024) NOT NULL DEFAULT \'\',
  "is_choice_constraint" integer NOT NULL DEFAULT \'0\',
  CONSTRAINT "option_constraint_ibfk_1" FOREIGN KEY ("option_id") REFERENCES "options" ("id") ON DELETE CASCADE
)
', 
'option_chice' => 'CREATE TABLE "option_choice" (
  "id" varchar(50) NOT NULL DEFAULT \'\',
  "option_id" varchar(50) NOT NULL DEFAULT \'\',
  "longname" varchar(50) DEFAULT NULL,
  "shortname" varchar(50) DEFAULT NULL,
  "driverval" text,
  CONSTRAINT "option_choice_ibfk_1" FOREIGN KEY ("option_id") REFERENCES "options" ("id") ON DELETE CASCADE
)',
'option_choice_translation' => 'CREATE TABLE "option_choice_translation" (
  "id" varchar(50) NOT NULL DEFAULT \'\',
  "option_id" varchar(50) NOT NULL DEFAULT \'\',
  "lang" varchar(8) NOT NULL DEFAULT \'\',
  "longname" varchar(50) DEFAULT NULL,
  CONSTRAINT "option_choice_translation_ibfk_1" FOREIGN KEY ("id") REFERENCES "option_choice" ("id") ON DELETE CASCADE,
  CONSTRAINT "option_choice_translation_ibfk_2" FOREIGN KEY ("option_id") REFERENCES "option_choice" ("option_id") ON DELETE CASCADE
)',
'driver' => 'CREATE TABLE "driver" (
  "id" varchar(50) NOT NULL,
  "name" varchar(50) NOT NULL,
  "driver_group" varchar(50) DEFAULT NULL,
  "locales" text,
  "obsolete" varchar(50) DEFAULT NULL,
  "pcdriver" varchar(8) DEFAULT NULL,
  "url" tinytext DEFAULT \'\',
  "supplier" varchar(50) DEFAULT NULL,
  "thirdpartysupplied" integer NOT NULL DEFAULT \'1\',
  "manufacturersupplied" tinytext,
  "license" text,
  "licensetext" text,
  "licenselink" tinytext,
  "nonfreesoftware" integer DEFAULT NULL,
  "patents" integer DEFAULT NULL,
  "shortdescription" text,
  "max_res_x" integer DEFAULT NULL,
  "max_res_y" integer DEFAULT NULL,
  "color" integer DEFAULT NULL,
  "text" integer DEFAULT NULL,
  "lineart" integer DEFAULT NULL,
  "graphics" integer DEFAULT NULL,
  "photo" integer DEFAULT NULL,
  "load_time" integer DEFAULT NULL,
  "speed" integer DEFAULT NULL,
  "execution" varchar(255) NOT NULL DEFAULT \'ghostscript\',
  "no_pjl" integer NOT NULL DEFAULT \'0\',
  "no_pageaccounting" integer NOT NULL DEFAULT \'0\',
  "prototype" text,
  "pdf_prototype" text,
  "ppdentry" text,
  "comments" text
)',
'driver_translation' => 'CREATE TABLE "driver_translation" (
  "id" varchar(50) NOT NULL,
  "lang" varchar(8) NOT NULL DEFAULT \'\',
  "supplier" varchar(50) DEFAULT NULL,
  "license" text,
  "licensetext" text,
  "licenselink" tinytext,
  "shortdescription" text,
  "comments" text,
  CONSTRAINT "driver_translation_ibfk_1" FOREIGN KEY ("id") REFERENCES "driver" ("id") ON DELETE CASCADE
)',
'driver_support_contact' => 'CREATE TABLE "driver_support_contact" (
  "driver_id" varchar(50) NOT NULL,
  "url" varchar(255) NOT NULL,
  "level" varchar(20) NOT NULL,
  "description" text,
  CONSTRAINT "driver_support_contact_ibfk_1" FOREIGN KEY ("driver_id") REFERENCES "driver" ("id") ON DELETE CASCADE
)',
'driver_support_conctact_translation' => 'CREATE TABLE "driver_support_contact_translation" (
  "driver_id" varchar(50) NOT NULL,
  "url" varchar(255) NOT NULL,
  "level" varchar(20) NOT NULL,
  "lang" varchar(8) NOT NULL DEFAULT \'\',
  "description" text,
  CONSTRAINT "driver_support_contact_translation_ibfk_1" FOREIGN KEY ("driver_id", "url", "level") REFERENCES "driver_support_contact" ("driver_id", "url", "level") ON DELETE CASCADE
)',
'driver_dependency' => 'CREATE TABLE "driver_dependency" (
  "driver_id" varchar(50) NOT NULL,
  "required_driver" varchar(50) NOT NULL,
  "version" varchar(50) DEFAULT NULL,
  CONSTRAINT "driver_dependency_ibfk_1" FOREIGN KEY ("driver_id") REFERENCES "driver" ("id") ON DELETE CASCADE
)',
'driver_package' => 'CREATE TABLE "driver_package" (
  "driver_id" varchar(50) NOT NULL,
  "scope" varchar(255) NOT NULL,
  "fingerprint" varchar(767) NOT NULL DEFAULT \'\',
  "name" text,
  CONSTRAINT "driver_package_ibfk_1" FOREIGN KEY ("driver_id") REFERENCES "driver" ("id") ON DELETE CASCADE
)',
'margin' => 'CREATE TABLE "margin" (
  "driver_id" varchar(50) NOT NULL DEFAULT \'\',
  "printer_id" varchar(50) NOT NULL DEFAULT \'\',
  "margin_type" varchar(255) NOT NULL,
  "pagesize" varchar(50) NOT NULL DEFAULT \'\',
  "margin_unit" varchar(255) DEFAULT NULL,
  "margin_absolute" integer DEFAULT \'0\',
  "margin_top" float DEFAULT NULL,
  "margin_left" float DEFAULT NULL,
  "margin_right" float DEFAULT NULL,
  "margin_bottom" float DEFAULT NULL
)',
'driver_printer_assoc' => 'CREATE TABLE "driver_printer_assoc" (
  "driver_id" varchar(50) NOT NULL,
  "printer_id" varchar(50) NOT NULL,
  "comments" text,
  "max_res_x" integer DEFAULT NULL,
  "max_res_y" integer DEFAULT NULL,
  "color" integer DEFAULT NULL,
  "text" integer DEFAULT NULL,
  "lineart" integer DEFAULT NULL,
  "graphics" integer DEFAULT NULL,
  "photo" integer DEFAULT NULL,
  "load_time" integer DEFAULT NULL,
  "speed" integer DEFAULT NULL,
  "ppd" tinytext,
  "ppdentry" text,
  "pcomments" text,
  "fromdriver" integer NOT NULL DEFAULT \'0\',
  "fromprinter" integer NOT NULL DEFAULT \'0\'
)',
'driver_printer_assoc_translation' => 'CREATE TABLE "driver_printer_assoc_translation" (
  "driver_id" varchar(50) NOT NULL,
  "printer_id" varchar(50) NOT NULL,
  "lang" varchar(8) NOT NULL DEFAULT \'\',
  "comments" text,
  "pcomments" text,
  CONSTRAINT "driver_printer_assoc_translation_ibfk_1" FOREIGN KEY ("driver_id", "printer_id") REFERENCES "driver_printer_assoc" ("driver_id", "printer_id") ON DELETE CASCADE
)'
};

sub schema {
	return $schemaPhonebook;
}

1;
