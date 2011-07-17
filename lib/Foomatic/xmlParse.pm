package Foomatic::xmlParse;

use strict;
use warnings;
use Sys::Hostname;
use POSIX qw/strftime/;
#use Data::Dumper;
use XML::LibXML;
use Clone;
use Foomatic::phonebook;

### Code style ###
# - Tabs for indentation
# - Camel case
# - new block's opening braces on same line

#Constructor
sub new {
	my ($class, $langPref, $version) = @_;
	my $this = {};
	bless $this, $class;
	
	#default to <en>
	$this->{'langPref'} = "en";
	if($langPref) {
		$this->{"langPref"} = $langPref;
	}
	
	#default to 0, perfect compatablity with C primary xml parsing
	# 1 = compatibility of C combo parsing
	# 2 = multilingual support
	$this->{'version'} = 0;
	if($version) {
		$this->{'version'} = $version;
	}
	
	my $phonebook = Foomatic::phonebook->new($this->{'version'});
	$this->{'printerPhonebook'} = $phonebook->printer();
	$this->{'driverPhonebook'}  = $phonebook->driver();
	$this->{'optionPhonebook'}  = $phonebook->option();
	$this->{'comboPhonebook'}   = $phonebook->combo();
	return $this;
}

sub cleanID {
	my ($id) = @_;
	$id =~ s/^[^\/]*\///;
	#remove everything before the leading slash
	return $id;
}


#xml parser
my $parser = XML::LibXML->new();



sub generalGroups {
	my ($this, $group, $perlData, $destinationKey, $node) = @_;
	if($group > 10) {
		return 0;#option is not general
	} elsif($group == 1) {
		my $value = $node->to_literal;
		if(!($value eq '')) {$$perlData{$$destinationKey} = $value;}
		#basic "take node text and put into hash"
		
	} elsif ($group == 2) {
		$$perlData{$$destinationKey} = 1;
		#if the node was found set key to true
		
	} elsif ($group == 3) {
		$$perlData{$$destinationKey} = 0;
		#if node was found set key to false
		
	} elsif ($group == 4) {
		$$perlData{$$destinationKey} = cleanID( $node->to_literal );
		#ID
		
	} elsif ($group == 5) {
		$$perlData{$$destinationKey} = {};
		$this->setMargins($perlData,$destinationKey, $node);
	} elsif($group == 6) {
		$this->setHumanReadableText($perlData,$destinationKey, $node);
	} elsif($group == 7) {
		my $value = $node->to_literal;
		$value =~ s/^\s+//;
		$value =~ s/\s+$//;
		if($value) {$$perlData{$$destinationKey} = $value;}
		
	} else {
		return 0;#the group was not general
	}
	return 1;#the group was general
}

sub setHumanReadableText {
	my ($this, $perlData, $destinationKey, $node) = @_;
	my %humanTexts;
	
	foreach my $subnode ($node->findnodes("./*")) {
		my $lang = $subnode->nodeName();
		$humanTexts{$lang} = $subnode->to_literal();
	}
	
	if( $humanTexts{$this->{"langPref"}} ) {
		#legacy multi-lingual, only the preffered language, which defaults to en
		$$perlData{$$destinationKey} = $humanTexts{$this->{"langPref"}}; 
	} elsif($humanTexts{"en"}) {
		$$perlData{$$destinationKey} = $humanTexts{"en"}; 
	}

	#full multi-lingual, all the avalible languages
	if($this->{'version'} > 1) {
		$$perlData{$$destinationKey . 's'} = \%humanTexts;
	}
}

sub setMargins {
	my ($this, $perlData, $destinationKey, $node) = @_;

	$$perlData{$$destinationKey} = {};
	foreach my $subnode ($node->findnodes("./*")) {
		my $name = $subnode->nodeName();
		if($name eq "general") {
			$name = "_".$name;
		} else {
			foreach my $attribute ($subnode->findnodes("./\@PageSize")) {
				$name = $attribute->to_literal();
			}
		}
		
		my %margin;
		$margin{"absolute"} = "1";
		foreach my $subsubnode ($subnode->findnodes("./*")) {#childern nodes
			my $key = $subsubnode->nodeName();
			if ($key eq "relative") {
				$margin{"absolute"} = "0";
			} else {
				$margin{$key} = $subsubnode->to_literal();
			}
		}
		
		$$perlData{$$destinationKey}->{$name} = \%margin;
	}
}

#PRINTER XML

sub parsePrinter {
	my($this, $xmlPath) = @_;
	die unless $xmlPath;
	my %perlData = %{defaultPrinterData()};
	
	
	my $tree = $parser->parse_file($xmlPath);
	my $root = $tree->getDocumentElement;
	
	my $xpath;
	my $destinationKey;
	my $group;
	
	foreach my $nodeEntry (@{$this->{'printerPhonebook'}}) {
		($xpath, $destinationKey, $group) = @{$nodeEntry};
		
		foreach my $node ($root->findnodes($xpath)) {#takes the last node if multiple found
			#See phonebook.pm for group lookup table
			#The general groups
			if( $this->generalGroups($group, \%perlData, \$destinationKey, $node) ){
				
			#The specific groups
			} elsif ($group == 12) {#drivers
				foreach my $subnode ($node->findnodes("./driver")) {
					my %driver;
					foreach my $driverID ($subnode->findnodes("./id[1]")) {
						$driverID = $driverID->to_literal;
						$driver{"name"} = $driverID;
						$driver{"id"} = $driverID;
					}
					foreach my $ppd ($subnode->findnodes("./ppd[1]")) {
						$driver{"ppd"} = $ppd->to_literal;
					}
					foreach my $comments ($subnode->findnodes("./comments")) {
						$this->setHumanReadableText(\%driver,\"comment", $comments);
					}
					push(@{$perlData{$destinationKey}}, \%driver);
				}
				
			} elsif ($group == 13) {#lang
				foreach my $subnode ($node->findnodes("./*")) {
					my $name = $subnode->nodeName();
					if ($name eq "text" || $name eq "pjl") {next()};
					
					my %lang;
					$lang{"level"} = "";
					foreach my $level ($subnode->findnodes("./\@level")) {#selects the "level" attribute itself
						$lang{"level"} = $level->to_literal;
					}
					$lang{"name"} = $name;
					push(@{$perlData{$destinationKey}}, \%lang);
				}
				
			} elsif ($group == 14) {#printer types
				$perlData{$destinationKey} = $node->nodeName;
				
			}
		}
	}
	
	return \%perlData;
}

sub defaultPrinterData {
	return {
		'unverified' => '0',
		'noxmlentry' => '0',
		'ppdentry'   => undef, #this appears to be outdated in favour of "lang"
		'color'      => '0',
		'pjl'        => '0',
		'functionality' => 'X',
	}
}

#DRIVER XML

sub parseDriver {
	my($this, $xmlPath) = @_;
	die unless $xmlPath;
	my %perlData = %{defaultDriverData()};
	
	my $tree = $parser->parse_file($xmlPath);
	my $root = $tree->getDocumentElement;
	
	my $xpath;
	my $destinationKey;
	my $group;
	
	foreach my $nodeEntry (@{$this->{'driverPhonebook'}}) {
		($xpath, $destinationKey, $group) = @{$nodeEntry};
		
		foreach my $node ($root->findnodes($xpath)) {
			#The general groups
			if( $this->generalGroups($group, \%perlData, \$destinationKey, $node) ){
				
			#The specific groups
			} elsif ($group == 11) {#type
				my $type = $node->nodeName;
				$type = uc(substr($type, 0, 1));
				$perlData{$destinationKey} = $type;
			
			} elsif ($group == 12) {#printers
				foreach my $subnode ($node->findnodes("./printer")) {
					my %printer;
					
					foreach my $subsubnode ($subnode->findnodes("./id")) {
						my $key = $subsubnode->nodeName();
						$printer{$key} = cleanID( $subsubnode->to_literal );
					}
					
					foreach my $comments ($subnode->findnodes("./comments")) {
						$this->setHumanReadableText(\%printer,\"comment", $comments);
					}
					
					if($this->{'version'} > 0) {
						foreach my $margins ($subnode->findnodes("./margins")) {
							$this->setMargins(\%printer,\"margins", $margins);
						}
					}
					
					foreach my $subsubnode ($subnode->findnodes("./functionality/*")) {
						my $key = $subsubnode->nodeName();
						my $value = $subsubnode->to_literal();
						
						if($key eq "monochrome"){$key="color";$value=0}
						$key = "exc".$key;
						
						$printer{$key} = $value;
					}


					push(@{$perlData{$destinationKey}}, \%printer);
					if ($this->{'version'} > 1) {
						$perlData{$destinationKey.'_byname'}->{$printer{'id'}} = \%printer ;
					}
				}
				
			} elsif ($group == 13) {#support contact
				foreach my $subnode ($node->findnodes("./supportcontact")) {
					my %contact;

					foreach my $subsubnode ($subnode->findnodes('./@level | ./@url')) {
						my $key = $subsubnode->nodeName();
						$contact{$key} = $subsubnode->to_literal();
					}

					$contact{"description"} = $subnode->to_literal();
					push(@{$perlData{$destinationKey}}, \%contact);
				}
				
			} elsif ($group == 14) {#packages
				foreach my $subnode ($node->findnodes("./package")) {
					my %package;

					foreach my $subsubnode ($subnode->findnodes('./@scope | ./@fingerprint')) {
						my $key = $subsubnode->nodeName();
						$package{$key} = $subsubnode->to_literal();
					}

					$package{"url"} = $subnode->to_literal();
					push(@{$perlData{$destinationKey}}, \%package);
				}
				
			} elsif ($group == 15) {#license link
				my $pref = $this->{"langPref"};
				my $link;
				foreach my $subnode ($node->findnodes("./en/\@url[1]")) {
					$link = $subnode->to_literal();
				}
				if($pref ne 'en') {
					foreach my $subnode ($node->findnodes("./$pref/\@url[1]")) {
						$link = $subnode->to_literal();
					}
				}
				if ($link) {
					$perlData{$destinationKey} = $link;
					$perlData{'orig' . $destinationKey} = $link;
				}
			}
		}
	}
	
	return \%perlData;
}

sub defaultDriverData {
	return {
		'ppdentry'   => undef,
		'printers'   => [],
	}
}

#OPTIONS XML

sub setConstraint {#complex type used twice in option xmls
	my ($node, $perlData) = @_;
	foreach my $subnode ($node->findnodes("./constraint")) {
		my %constraint;

		foreach my $subsubnode ($subnode->findnodes('./*')) {
			my $key = $subsubnode->nodeName;
			$constraint{$key} = $subsubnode->to_literal();
		}

		foreach my $subsubnode ($subnode->findnodes('./@sense')) {
			my $boolean = $subsubnode->to_literal();
			if ($boolean eq "false") {
				$boolean = 0;
			} else {
				$boolean = 1;
		}
			$constraint{"sense"} = $boolean;
		}

		push(@{$$perlData}, \%constraint);
	}
}

sub parseOption {
	my($this, $xmlPath) = @_;
	die unless $xmlPath;
	my %perlData = %{defaultOptionData()};
	
	my $tree = $parser->parse_file($xmlPath);
	my $root = $tree->getDocumentElement;
	
	my $xpath;
	my $destinationKey;
	my $group;
	
	foreach my $nodeEntry (@{$this->{'optionPhonebook'}}) {
		($xpath, $destinationKey, $group) = @{$nodeEntry};
		
		foreach my $node ($root->findnodes($xpath)) {
			#The general groups
			if( $this->generalGroups($group, \%perlData, \$destinationKey, $node) ){
				
			#The specific groups
			} elsif ($group == 11) { #constraints
				setConstraint($node, \$perlData{$destinationKey});
				
			} elsif ($group == 12) { #enum_values
				foreach my $subnode ($node->findnodes("./enum_val")) {
					my %enumValue;
					
					foreach my $subsubnode ($subnode->findnodes('./@id[1]')) {
						$enumValue{"idx"} = $subsubnode->to_literal;
					}
					
					foreach my $longnames ($subnode->findnodes('./ev_longname')) {
						$this->setHumanReadableText(\%enumValue,\"comment", $longnames);
					}
					
					foreach my $subsubnode ($subnode->findnodes('./ev_shortname')) {
						my $shortname = $subsubnode->to_literal;
						$shortname =~ s!^\s*!!;
						$shortname =~ s!\s*$!!;
						$enumValue{"value"} = $shortname;
					}
					
					foreach my $subsubnode ($subnode->findnodes('./ev_driverval')) {
						$enumValue{"driverval"} = $subsubnode->to_literal;
					}

					foreach my $subsubnode ($subnode->findnodes('./constraints')) {
						setConstraint($subsubnode, \$enumValue{"constraints"});
					}

					push(@{$perlData{$destinationKey}}, \%enumValue);
				}
				
			} elsif ($group == 13) {#Style
				#Style is odd, it needs 
				my $style = $node->nodeName();
				if ($style eq 'arg_postscript') {
					$style = 'G';
				} elsif ($style eq 'arg_substitution') {
					$style = 'C';
				} elsif ($style eq 'arg_pjl') {
					$style = 'J';
				} elsif ($style eq 'arg_forced_composite') {
					$style = 'F';
				}
				$perlData{$destinationKey} = $style;
			}
		}
	}
	
	return \%perlData;
}

sub defaultOptionData {
	return {
		'vals' => [],
		'style' => 'X',
	}
}


#OVERVIEW
#TODO: Move the nodestokeep lists into the phonebook

sub printerNodesToKeep {
	return [
	'make',
	'model',
	'id',
	'drivers',
	'general_mdl',
	'general_ieee',
	'general_mfg',
	'general_des',
	'general_cmd',
	"snmp_des",
	"snmp_mfg",
	"snmp_mdl",
	"snmp_ieee",
	"snmp_cmd",
	"usb_des",
	"usb_mfg",
	"usb_mdl",
	"usb_ieee",
	"usb_cmd",
	"par_des",
	"par_mfg",
	"par_mdl",
	"par_ieee",
	"par_cmd",
	'functionality',
	'driver',
	'drivers',
	'unverified'];
}

sub driverNodesToKeep {
	return [
	'manufacturersupplied',
	'drvcolor',
	'lineart',
	'supportcontacts',
	'shortdescription',
	'license',
	'graphics',
	'speed',
	'text',
	'supplier',
	'url',
	'type',
	'drvmaxresy',
	'drvmaxresx',
	'supplier',
	'photo',
	'printers',
	'printers_byname',
	'packages',
	'origlicensetext',
	'licensetext',
	'obsolete',
	'patents'];
}

sub nodesToDelete {
	#We subtract the wanted nodes from the phonebook, leaving the unwanted nodes
	my ($this, $wantedNodes, $phonebook) = @_;
	my @unwantedNodes;
	
	foreach my $nodeEntry (@{$phonebook}) {
		my ($xpath, $destinationKey, $group) = @{$nodeEntry};
		if( !grep(/^$destinationKey/, @{$wantedNodes}) ) {
			push(@unwantedNodes, $destinationKey);
		}
	}
	
	return @unwantedNodes;
}

sub getDriverPrinter {
	my ($this, $driver, $printerID) = @_;
	my $drvPrinter = 0;
	if($driver->{'printers_byname'}) {#version 2 or better
		$drvPrinter = $driver->{'printers_byname'}{$printerID};
	} elsif ($driver->{'printers'}) {#much slower fall back
		foreach my $hash (@{$driver->{'printers'}}) {
			if ($hash->{'id'} && $hash->{'id'} eq $printerID) {
				$drvPrinter = $hash;
				last;
			}
		}
	}
	return $drvPrinter;
}

sub getPrinterSpecificDriver {
	my ($this, $driver, $printer, $nonDestructive) = @_;
	my $printerName = $printer->{'id'};
	
	
	#A printer might have a lower dpi than the driver's default
	#Use the lowwest, can be further overwriten by pair specific data.
	my $overrideX = 0;
	if(defined($printer->{'maxxres'}) && defined($driver->{'drvmaxresx'})
	 && $printer->{'maxxres'} < $driver->{'drvmaxresx'}) {
		$overrideX = 1;
	} #X cordanate
	my $overrideY = 0;
	if(defined($printer->{'maxyres'}) && defined($driver->{'drvmaxresy'})
	 && $printer->{'maxyres'} < $driver->{'drvmaxresy'}) {
		$overrideX = 1;
	} #Y cordanate
	
	#Find the printer inside the original driver
	my $drvPrinter = $this->getDriverPrinter($driver, $printerName);
	my $specificDriver;
	
	#Do we need to customize the driver data, making a clone
	#or is the original data ok?
	if( $overrideX || $overrideY || ($drvPrinter && (
		defined $drvPrinter->{'excmaxresy'}  ||
		defined $drvPrinter->{'excmaxresx'}  ||
		defined $drvPrinter->{'exccolor'}    ||
		defined $drvPrinter->{'excgraphics'} ||
		defined $drvPrinter->{'exclineart'}  ||
		defined $drvPrinter->{'exctext'}     ||
		defined $drvPrinter->{'excphoto'} )) ) {
		
		$specificDriver = Clone::clone($driver);
		
		#Overview needs this data removed in a clone
		delete $specificDriver->{'printers'} if (!$nonDestructive);
		
		if($overrideX) {
			$specificDriver->{'drvmaxresx'} = $printer->{'maxxres'};
		}
		if($overrideY) {
			$specificDriver->{'drvmaxresy'} = $printer->{'maxyres'};
		}
		
		#source key, key to overwrite with printer specific data, if present
		my @overrides = ( 
		['excmaxresy', 'drvmaxresy'],
		['excmaxresx', 'drvmaxresx'],
		['exccolor', 'drvcolor'],
		['excgraphics', 'graphics'],
		['exclineart', 'lineart'],
		['exctext', 'text'],
		['excphoto', 'photo'],
		['margins', 'combomargins']);
		
		foreach my $keys (@overrides) {
			if($drvPrinter && defined( $drvPrinter->{$keys->[0]} ) ) {
				$specificDriver->{$keys->[1]} =  $drvPrinter->{$keys->[0]};
				
			}
		}
	} else {#The general driver is the specific one
		$specificDriver = $driver;
	}

	#overview needs the original priner specific data removed
	# so that it does not get reprocessed
	if($drvPrinter && !$nonDestructive) { 
		$drvPrinter->{'id'} = undef; #We clean out the array later,
		#Quick and dirty delete.
	}
	
	return $specificDriver;
}

sub parseOverview {
	my ($this, $printerXMLs, $driverXMLs, $skipPPDs) = @_;
	$printerXMLs  || die("Need Printer XMLs\n");
	$driverXMLs  || die("Need Driver XMLs\n");
	$skipPPDs = 0 if(!defined($skipPPDs));

	
	#BULK READ ALL PRINTERS
	#read all printers into hash with printer id as key
	my @unwantedPrinterNodes = $this->nodesToDelete($this->printerNodesToKeep(), $this->{'printerPhonebook'});
	my %printers;
	foreach my $printerXML (@$printerXMLs) {
		my $printerPerlData = $this->parsePrinter($printerXML);
		
		#delete unused nodes
		foreach my $node (@unwantedPrinterNodes) {
			if (exists $printerPerlData->{$node}) {
				delete $printerPerlData->{$node};
			}
		}
		
		$printers{$printerPerlData->{'id'}} = $printerPerlData;
	}
	
	
	#BULK READ ALL DRIVERS
	#read all drivers into hash with name as key
	my @unwantedDriverNodes = $this->nodesToDelete($this->driverNodesToKeep(), $this->{'driverPhonebook'});
	my %drivers;
	foreach my $driverXML (@$driverXMLs) {
		$driverXML =~ m!/([^/]*).xml$!;
		my $id =  $1;
		
		my $driverPerlData = $this->parseDriver($driverXML);
		
		#delete unused nodes
		foreach my $node (@unwantedDriverNodes) {
			if (exists $driverPerlData->{$node}) {
				delete $driverPerlData->{$node};
			}
		}
		
		$drivers{$id} = $driverPerlData;
	}
	

	#ADD DRIVER DATA TO PRINTER
	foreach my $printer (keys %printers) {
		$printer = $printers{$printer};
		
		my $printerDrivers = $printer->{'drivers'};
		$printer->{'drivers'} = []; #key gets reused
		
		foreach my $printerDriver (@{$printerDrivers}) {
			if(defined($printerDriver->{'ppd'}) && $skipPPDs) { #cupsppd option, skipPPDs
				my $driverPrinter = $this->getDriverPrinter($drivers{$printerDriver->{'id'}}, $printer->{'id'});
				$driverPrinter = undef;
				$printerDriver = undef;
			} else {
				push(@{$printer->{'drivers'}}, $printerDriver->{'id'});
			}
		}

		#C program behaviour
		$printer->{'driverproperties'} = {} if ($this->{'version'} == 1 && $printerDrivers);
		
		#add driver data to printer in memory
		foreach my $driver ( @{$printerDrivers} ) {
			next if (!defined($driver));#This pair was skipped because of skipPPDs
			
			my $id = $driver->{'id'};
			
			#ADD DRIVER DATA
			if ($driver->{'ppd'}) {
				#A printer specific ppd is avalible
				my %ppd;
				$ppd{'ppdfile'} = $driver->{'ppd'};
				$ppd{'driver'} = $id;
				push( @{$printer->{'ppds'}}, \%ppd );
			}
			
			#only add the driver data if the driver is in memory
			#TODO: Figure out why it only works with the "->{'type'}"
			if (defined($drivers{$id}->{'type'})) {
				$printer->{'driverproperties'}{$id} = $this->getPrinterSpecificDriver($drivers{$id}, $printer, 0);
			}
		}
	}
	

	#ADD DRIVER DATA TO PRINTERS WITHOUT XML ENTRIES
	foreach my $driverName (keys %drivers) {
		my $driver = $drivers{$driverName};
		foreach my $printer (@{$driver->{'printers'}}) {
			if($printer->{'id'}) {
				my $id = $printer->{'id'};
				if (!$printers{$id}) {#create new no xml printer
					my ($make, $model);
					if ($id =~ /^([^\-]+)\-(.*)$/) {
						$make = $1;
						$model = $2;
						$make =~ s/_/ /g;
						$model =~ s/_/ /g;
					} else {
						$make = $id;
						$make =~ s/_/ /g;
						$model = "Unknown model";
					}
					$printers{$id} = {
						'id' => $id,
						'make' => $make,
						'model' => $model,
						'functionality' => 'X',
						'unverified' => '0',
						'noxmlentry' => 1
					}
				}
				
				#add driver data to printer
				$printers{$id}->{'driverproperties'}{$driverName} = $this->getPrinterSpecificDriver($driver, $printer, 0);
			}
		}
		delete $driver->{'printers'}; #cleans out the driver's printers array
	}
	
	#Create array out of printers hash
	my @overview;
	foreach my $pid (keys %printers) {
		push(@overview, $printers{$pid});
	}
	return \@overview;
}

#COMBO DATA

my %driverCache;
my %optionCache;
my %optionRelations;

sub getOptionRelationships {
	my ($this, $optionXMLs) = @_;
	die("Need option XMLs\n") if (!$optionXMLs);
	
	if ( keys(%optionRelations) > 0 ) {
		#cache is non empty, use it
		return \%optionRelations;
	}
	
	foreach my $optionXML (@$optionXMLs) {
		my $optionData = $this->parseOption($optionXML);
		$optionCache{$optionData->{'idx'}} = $optionData;
	}
	
	foreach my $option (keys %optionCache) {
		foreach my $constraint (@{ $optionCache{$option}{'constraints'} }) {
			my $default = undef;
			$default = $constraint->{'arg_defval'} if defined($constraint->{'arg_defval'});
			
			my $printer = '*';
			my $driver = '*';
			my $make = '*';
			if(defined($constraint->{'printer'})) {
				$printer = cleanID($constraint->{'printer'});
			} 
			if(defined($constraint->{'driver'})) {
				$driver = $constraint->{'driver'};
			}
			if(defined($constraint->{'make'})) {
				$make = $constraint->{'make'};
			}
			
			$optionRelations{$make}{$printer}{$driver}{$option} = [$constraint->{'sense'}, $default];
		}
		#Clean out constraints
		delete $optionCache{$option}->{'constraints'};
	}
	return \%optionRelations;
}

sub getOptions{
	my ($this, $relations, $make, $printerID, $driverID) = @_;
	#Get a list of the supported options for this driver printer pair
	#includes the sense and default value
	
	#Entries get progressivly overwritten by more specific ones
	my %supportedOptions;
	if(defined($relations->{'*'}{'*'}{'*'})) {
		%supportedOptions = (%supportedOptions , %{$relations->{'*'}{'*'}{'*'}});
	}
	if(defined($relations->{'*'}{'*'}{$driverID})) {
		%supportedOptions = (%supportedOptions , %{$relations->{'*'}{'*'}{$driverID}});
	}
	if(defined($relations->{'*'}{$printerID}{'*'})) {
		%supportedOptions = (%supportedOptions , %{$relations->{'*'}{$printerID}{'*'}});
	}
	if(defined($relations->{'*'}{$printerID}{$driverID})) {
		%supportedOptions = (%supportedOptions , %{$relations->{'*'}{$printerID}{$driverID}});
	}
	if(defined($relations->{$make}{'*'}{$driverID})) {
		%supportedOptions = (%supportedOptions , %{$relations->{$make}{'*'}{$driverID}});
	}
	if(defined($relations->{$make}{$printerID}{'*'})) {
		%supportedOptions = (%supportedOptions , %{$relations->{$make}{$printerID}{'*'}});
	}
	if(defined($relations->{$make}{$printerID}{$driverID})) {
		%supportedOptions = (%supportedOptions , %{$relations->{$make}{$printerID}{$driverID}});
	}
	
	return \%supportedOptions;
}

sub isValueSupported{
	my ($this, $make, $printerID, $driverID, $value) = @_;
	$printerID = 'printer/'.$printerID;
	#I'm borrowing the concept of specifity from css. Used to determine which
	#constraint's sense is most specific to the driver printer pair. 
	
	#0 default sense, true
	#2 driver default
	#4 printer default
	#6 driver/printer pair explicitly
	#7 driver/printer/make pair explicitly
	my $specifity = 0;
	my $sense = 1;
	
	if (defined($value->{'constraints'})) {
		foreach my $constraint (@{$value->{'constraints'}}) {
			my $localSpecifity = 0;
			
			#Calculate how specific this sense is
			if (defined($constraint->{'printer'})) {
				if ($constraint->{'printer'} eq $printerID) {
					$localSpecifity += 4;
				} else {$localSpecifity = -10;}
			}
			if (defined($constraint->{'driver'})) {
				if($constraint->{'driver'} eq $driverID) {
					$localSpecifity += 2;
				} else {$localSpecifity = -10;}
			}
			if (defined($constraint->{'make'})) {
				if($constraint->{'make'} eq $make) {
					$localSpecifity += 1;
				} else {$localSpecifity = -10;}
			}
			
			#If we've found a more specific sense then assign it
			if($localSpecifity > $specifity) {
				$sense = $constraint->{'sense'};
				$specifity = $localSpecifity;
			}
		}
		delete $value->{'constraints'};#Yes that's correct, a side-effect
	}
	
	return $sense;
}

sub setPPDFile {
	my ($this, $structure, $printer, $driver) = @_;
	
	my $ppd;
	my $driverName = $driver->{'name'};
	
	foreach my $printerDriver (@{$printer->{'drivers'}}) {
		if($printerDriver->{'name'} eq $driverName) {
			if (defined($printerDriver->{'ppd'})) {
				$ppd = $printerDriver->{'ppd'};
			}
			last; #we've found what we're looking for
		}
	}
	
	$structure->{'ppdfile'} = $ppd if(defined($ppd));
}

sub parseCombo {
	my ($this, $printerPath, $driverPath, $optionXMLs) = @_;
	die if (! (-r $printerPath));#printer and driver xmls must exist
	die if (! (-r $driverPath));
	 
	my $combo = $this->defaultComboData();
	my $printer = $this->parsePrinter($printerPath);
	
	my $driver;#In memory driver cache
	if(!defined($this->{'driverCache'}{$driverPath}) {
		$driver = $this->parseDriver($driverPath);
		$this->{'driverCache'}{$driverPath} = $driver;
	} else {
		$driver = $this->{'driverCache'}{$driverPath};
	}
	
	$driver = $this->getPrinterSpecificDriver($driver, $printer, 1);
	$this->setPPDFile($combo, $printer, $driver);
	
	#option relationships, and option cache
	my $relationships = $this->getOptionRelationships($optionXMLs);
	
	#printer entries
	foreach my $keys (@{$this->{'comboPhonebook'}->{'printer'}}) {
		my ($source, $destination) = @{$keys};
		$destination = $source if (!$destination);
		
		if( defined($printer->{$source}) ) {
			$combo->{$destination} = $printer->{$source};
		}
	}
	
	#driver entries
	foreach my $keys (@{$this->{'comboPhonebook'}->{'driver'}}) {
		my ($source, $destination) = @{$keys};
		$destination = $source if (!$destination);
		
		if( defined($driver->{$source}) ) {
			$combo->{$destination} = $driver->{$source};
		}
		

	}
	
	#option entries
	my $printerName = $combo->{'id'};
	my $driverName = $combo->{'driver'};
	my $make = $combo->{'make'};
	
	my $driverPrinterOptions = $this->getOptions($relationships, $make, $printerName, $driverName);
	foreach my $option (keys %{$driverPrinterOptions}) {
		#check that the relationship supports this pair
		my ($sense, $defaultVal) = @{$driverPrinterOptions->{$option}};
		if ($sense) {
			#PJL options are not added if the driver does not support them
			if($combo->{'drivernopjl'}) {
				if($optionCache{$option}->{'style'} eq 'J') {
					next();}}
			
			$option = Clone::clone($optionCache{$option});
			
			#Maxspot, largest option spot we encounter
			if(defined($option->{'spot'}) && $option->{'spot'} gt $combo->{'maxspot'}) {
				$combo->{'maxspot'} = $option->{'spot'};
			} 
			
			
			#only include supported values for this option
			my $defaultFound = 0;
			my @supportedValues;
			foreach my $value (@{$option->{'vals'}}) {
				if( $this->isValueSupported($make, $printerName, $driverName, $value)) {
					
					#Driver/Printer default value for this option
					if ($value->{'idx'} eq $defaultVal) {
						$option->{'default'} = $value->{'value'};
						$defaultFound = 1;
					}
					
					push(@supportedValues, $value);
					$option->{'vals_byname'}{$value->{'value'}} = $value
				}
			}
			$option->{'vals'} = \@supportedValues;
			
			#If the  value specified by default value does not exist add
			#default value as is.
			if(!$defaultFound && defined($defaultVal)) {
				$option->{'default'} = $defaultVal;
			}
			
			#Add option to the args array and the args_by_name hash.
			push( @{$combo->{'args'}}, $option);
			$combo->{'args_byname'}{$option->{'name'}} = $option;
		}
	}
	
	return $combo;
}

sub defaultComboData {
	return {
		'url' => undef,
		'comboppdentry' => undef,
		'model' => undef,
		'make' => undef,
		'maxspot' => 'A',
		'driverppdentry' => undef,
		'compiled-at' => strftime('%Y-%m-%d %H:%M',localtime),
		'cmd' => undef,
		'id' => undef,
		'pjl' => undef,
		'type' => undef,
		'pcmodel' => undef,
		'printerppdentry' => undef,
		'manufacturersuppled' => undef,
		'recdriver' => undef,
		'args' => [],
		'args_byname' => {},
		'drivernopageaccounting' => 0,
		'compiled-by' => getlogin().'@'.hostname,
		'color' => undef,
		'timestamp' => time,
		'drivernopjl' => 0,
		'ascii' => 0, #appears to be unused
		'pcdriver' => undef,
		'comment' => undef,
		'driver' => undef,
		'pnp_ieee' => undef,
		'pnp_cmd' => undef,
		'pnp_mdl' => undef,
		'pnp_des' => undef,
		'pnp_mfg' => undef,
		'usb_ieee' => undef,
		'usb_cmd' => undef,
		'usb_mdl' => undef,
		'usb_des' => undef,
		'usb_mfg' => undef,
		'par_ieee' => undef,
		'par_cmd' => undef,
		'par_mdl' => undef,
		'par_des' => undef,
		'par_mfg' => undef,
		'snmp_ieee' => undef,
		'snmp_cmd' => undef,
		'snmp_mdl' => undef,
		'snmp_des' => undef,
		'snmp_mfg' => undef,
		'general_ieee' => undef,
		'general_cmd' => undef,
		'general_mdl' => undef,
		'general_des' => undef,
		'general_mfg' => undef,
	}
}

1;
