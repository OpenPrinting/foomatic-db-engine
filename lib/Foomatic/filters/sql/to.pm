package Foomatic::filters::sql::to;
use base ("Foomatic::filters::sql::sqlLayer");

use strict;
use warnings;
use Data::Dumper;

use DBI;
use Foomatic::filters::phonebook;
use Foomatic::util;

sub new( $ $ $ ) {
	my $class = shift;
	my $this = Foomatic::filters::sql::sqlLayer->new(@_);
	return bless($this, $class);
}


sub getConstraints {
	my ($this, $data, $optionId, $choiceId) = @_;
	
	my @constraints;
	foreach my $constraint (@{$data}) {
		my %preparedConstraint;
		$preparedConstraint{'option_id'} = getCleanId($optionId);
		
		$preparedConstraint{'choice_id'} = getCleanId($choiceId);
		
		$preparedConstraint{'driver'} = $constraint->{'driver'} 
			if($constraint->{'driver'});
		
		$preparedConstraint{'printer'} = $constraint->{'make'} . '-' 
			if($constraint->{'make'});
			
		$preparedConstraint{'printer'} = getCleanId($constraint->{'printer'}) 
			if($constraint->{'printer'});
		
		$preparedConstraint{'defval'} = $constraint->{'arg_defval'} 
			if(defined($constraint->{'arg_defval'}));
			
		if($constraint->{'sense'}) {
			#correct, the schema uses strings not booleans =\
			#or even ints
			#The reason for this is because the original implmentation
			#for pushing to this sql schema was written in php, I think.
			$preparedConstraint{'sense'} = 'true';
		} else {
			$preparedConstraint{'sense'} = 'false';
		}
		
		$preparedConstraint{'is_choice_constraint'} = 1 if($choiceId);
		
		push(@constraints, \%preparedConstraint);
	}
	
	return \@constraints;
}

sub getMargins {
	my ($this, $data, $driverId, $printerId) = @_;

	my @margins;
	foreach my $margin (keys %{$data} ) {
		my %margin;
		$margin{'driver_id'} = $driverId;
		$margin{'printer_id'} = $printerId;
		
		#margin type
		if($margin eq '_general') {
			$margin{'margin_type'} = 'general';
		} else {
			$margin{'margin_type'} = 'exception';
			$margin{'pagesize'} = $margin;
			
		}
		
		#margin data
		my $marginPntr = $data->{$margin};
		foreach my $key (keys %{$marginPntr}) {
			$margin{'margin_'.$key} = $marginPntr->{$key};
		}
		
		push(@margins, \%margin);
	}
	
	
	return \@margins;
}

sub getAssocs {
	my ($this, $data, $fromDriver, $parentId) = @_;
	my @assocs;
	my @comments;
	my @margins;
	foreach my $assoc (@{$data}) {
		
		#associations
		my %assoc;
		if($fromDriver) {
			$assoc{'driver_id'} = $parentId;
			$assoc{'printer_id'} = $assoc->{'id'};
		} else {
			$assoc{'driver_id'} = $assoc->{'name'};
			$assoc{'printer_id'} = $parentId;
		}
		$assoc{'comments'} = $assoc->{'comment'} if defined($assoc->{'comment'});
		$assoc{'ppdentry'} = $assoc->{'ppdentry'} if defined($assoc->{'ppdentry'});
		$assoc{'ppd'} = $assoc->{'ppd'} if defined($assoc->{'ppd'});
		
		$assoc{'max_res_x'} = $assoc->{'excmaxresx'} if defined($assoc->{'excmaxresx'});
		$assoc{'max_res_y'} = $assoc->{'excmaxresy'} if defined($assoc->{'excmaxresy'});
		
		$assoc{'color'} = $assoc->{'exccolor'} if defined($assoc->{'exccolor'});
		$assoc{'text'} = $assoc->{'exctext'} if defined($assoc->{'exctext'});
		$assoc{'lineart'} = $assoc->{'exclineart'} if defined($assoc->{'exclineart'});
		$assoc{'graphics'} = $assoc->{'excgraphics'} if defined($assoc->{'excgraphics'});
		$assoc{'photo'} = $assoc->{'excphoto'} if defined($assoc->{'excphoto'});
		$assoc{'lineart'} = $assoc->{'exclineart'} if defined($assoc->{'exclineart'});
		$assoc{'speed'} = $assoc->{'excspeed'} if defined($assoc->{'excspeed'});
		
		
		$assoc{'fromdriver'} = int $fromDriver;
		$assoc{'fromprinter'} = int !$fromDriver;
		push(@assocs, \%assoc);
		
		#comments
		if(defined($assoc->{'comments'})) {
			foreach my $lang (keys %{ $assoc->{'comments'} }) {
				my %comment;
				$comment{'driver_id'} = $assoc{'driver_id'};
				$comment{'printer_id'} = $assoc{'printer_id'};
				$comment{'lang'} = $lang;
				$comment{'comments'} = $assoc->{'comments'}{$lang};
				push(@comments, \%comment);
			}
		}
		
		#margins
		if(defined($assoc->{'margins'})) {
			my $margins = $this->getMargins($assoc->{'margins'},$assoc{'driver_id'}, $assoc{'printer_id'});
			@margins = (@margins, @{$margins});
		}
	}
	return (\@assocs, \@comments, \@margins);
}

sub setPreparedData {
	my ($this, $data, $phonebook) = @_;
	#We do most of our work here. This function takes the foomatic perl data
	#structure and flattens things to match the database schema.
	
	
	#compute the namespace from the phonebook
	my $nodePath = $phonebook->[0][0];#the nodepath of the first element
	$nodePath =~ m!^/([^/]*)/!;
	my $namespace = $1;
		
	#We sort the data by table and rename if needed.
	foreach my $element (@{$phonebook}) {
		my ($ignore, $source, $mainGroup, $table, $destination, $minorGroup) = @{$element};
		$destination = $source if (!$destination);
		
		#Only sort if the table is defined and if the destination key has not 
		#been set, does not work for complex types 
		if($table && !exists $data->{'prepared'}{$table}{$destination}) {
			
			#The business loop, complex data gets flattened
			
			if($minorGroup == 3) {
				next() if !defined($data->{$source});
				if($namespace eq 'driver') {#inverse the boolean
					$data->{'prepared'}{$table}{$destination} = int !$data->{$source};
				}
			
			} elsif($minorGroup == 4) {
				next() if !defined($data->{$source});
				if($namespace eq 'option') {
					$data->{'prepared'}{$table}{$destination} = getCleanId($data->{$source});
				} elsif($namespace eq 'driver' && $data->{$source}) {
					$data->{'prepared'}{$table}{$destination} = $data->{$source};
				}
			
			} elsif($mainGroup == 5) {
				next() if !defined($data->{$source});
				if($namespace eq 'driver') {#margins
					my $margins = $this->getMargins($data->{$source},$data->{'name'}, '');
					$data->{'prepared'}{$table}{$destination} = $margins;
				} elsif($namespace eq 'printer') {
					my $margins = $this->getMargins($data->{$source},'', $data->{'id'});
					$data->{'prepared'}{$table}{$destination} = $margins;
				}
			
			} elsif($minorGroup == 7) {#multilingual comments
				foreach my $lang (keys %{$data->{$source}}) {
					if($namespace eq 'printer') {
						my %preparedComment;
						$preparedComment{'id'} = $data->{'id'};
						$preparedComment{'lang'} = $lang;
						$preparedComment{'comments'} = $data->{'comments'}{$lang};
						push(@{$data->{'prepared'}{$table}{$destination}}, \%preparedComment);
					} elsif ($namespace eq 'option') {
						my %preparedComment;
						$preparedComment{'id'} = getCleanId($data->{'idx'});
						$preparedComment{'lang'} = $lang;
						$preparedComment{'longname'} = $data->{'comments'}{$lang};
						push(@{$data->{'prepared'}{$table}{$destination}}, \%preparedComment);
					} elsif ($namespace eq 'driver') {
						my %preparedComment;
						$preparedComment{'id'} = $data->{'name'};
						$preparedComment{'lang'} = $lang;
						$preparedComment{'comments'} = $data->{'comments'}{$lang};
						$preparedComment{'licensetext'} = $data->{'licensetexts'}{$lang};
						$preparedComment{'licenselink'} = $data->{'licenselink'};#fake multilang
						$preparedComment{'supplier'} = $data->{'supplier'};#fake multilang
						push(@{$data->{'prepared'}{$table}{$destination}}, \%preparedComment);
					}
				}
				
			} elsif($mainGroup == 11) {
				next() if !defined($data->{$source});
				if($namespace eq 'option') {#option constraints
					my $constraints = $this->getConstraints($data->{$source},$data->{'idx'}, '');
					
					$data->{'prepared'}{$table}{$destination} = $constraints;
				} elsif ($namespace eq 'driver') {#type
					my $type = $data->{$source};
					
					$type = 'ghostscript' if($type eq 'G');
					$type = 'postscript' if($type eq 'P');
					$type = 'uniprint' if($type eq 'U');
					$type = 'filter' if($type eq 'F');
					$type = 'ijs' if($type eq 'I');
					$type = 'cups' if($type eq 'C');
					
					$data->{'prepared'}{$table}{$destination} = $type;
				}
				
			} elsif($mainGroup == 12) {
				next() if !defined($data->{$source});
				if($namespace eq 'option') {#choices
					
					#We must merge the choice constraints with the option constraints
					my $allConstraints;
					if (defined($data->{'prepared'}{'option_constraint'}{'constraints'})) {
						$allConstraints = $data->{'prepared'}{'option_constraint'}{'constraints'};
					} else {
						$allConstraints = []; 
					}
					
					my @choices = ();
					
					my @comments = ();
					
					foreach my $choice (@{$data->{$source}}) {
						#Choice constraints, go to option_constraint
						if (defined($choice->{'constraints'})) {
							my $constraints = $this->getConstraints($choice->{'constraints'},$data->{'idx'}, $choice->{'idx'});
							@{$allConstraints} = (@{$allConstraints}, @{$constraints});
						}
						
						#Choice data, goes to option_choice
						my %choice;
						$choice{'id'} = getCleanId($choice->{'idx'});
						$choice{'option_id'} = getCleanId($data->{'idx'});
						$choice{'longname'} = $choice->{'comment'}
							if $choice->{'comment'};
						$choice{'shortname'} = $choice->{'value'}
							if $choice->{'value'};
						$choice{'driverval'} = $choice->{'driverval'}
							if defined($choice->{'driverval'});
						
						push(@choices, \%choice);
						
						#choice comments
						foreach my $lang (keys %{$choice->{'comments'}}) {
							my %comment;
							$comment{'longname'} = $choice->{'comments'}{$lang};
							$comment{'lang'} = $lang;
							$comment{'id'} = getCleanId($choice->{'idx'});
							$comment{'option_id'} = getCleanId($data->{'idx'});
							push(@comments, \%comment);
						}
						
					}
					$data->{'prepared'}{$table}{$destination} = \@choices;
					
					$data->{'prepared'}{$table.'_translation'}{'comments'} = \@comments;
					
					#If there were no option constraints we must assign
					#our choice constraints to the constraints prepared data,
					#only if we were able to find a choice with constraints
					if(@{$allConstraints}) {
						$data->{'prepared'}{'option_constraint'}{'constraints'} = $allConstraints;
					}
				} elsif($namespace eq 'printer') {#drivers list in printers
					my ($assocs, $comments) = $this->getAssocs($data->{$source}, 0, $data->{'id'});
					$data->{'prepared'}{$table}{$destination} = $assocs;
					$data->{'prepared'}{$table.'_translation'}{'comments'} = $comments;
				} elsif($namespace eq 'driver') {
					my ($assocs, $comments, $margins) = $this->getAssocs($data->{$source}, 1, $data->{'name'});
					$data->{'prepared'}{$table}{$destination} = $assocs;
					$data->{'prepared'}{$table.'_translation'}{'comments'} = $comments;
					#merge any existing margins
					if(defined($data->{'prepared'}{'margin'}{'margins'})) {
						$data->{'prepared'}{'margin'}{'margins'} = 
							[@{$data->{'prepared'}{'margin'}{'margins'}}, @{$margins}];
					} elsif(@{$margins}) {
						$data->{'prepared'}{'margin'}{'margins'} = $margins;
					}
				}
				
			} elsif($mainGroup == 13) { 
				next() if !defined($data->{$source});
				if($namespace eq 'printer') { #Languages
					foreach my $lang (@{$data->{$source}}) {
						my $name = $lang->{'name'};
						$data->{'prepared'}{$table}{$name}  = 1;
						
						if(defined($lang->{'level'}) && !($name eq 'proprietary') ) {
							$data->{'prepared'}{$table}{$name.'_level'} = $lang->{'level'};
						}
					}
					
				} elsif ($namespace eq 'driver') { #support contact
					my %contact;#the contact for the non-translation table
					my @contacts;
					foreach my $contact (@{$data->{$source}}) {
						$contact{'driver_id'} = $data->{'name'};
						$contact{'level'} = $contact->{'level'};
						$contact{'url'} = $contact->{'url'};
						$contact{'description'} = $contact->{'description'};
						
						#a clone, for use in the translated table
						my %multilangContact = %contact;
						
						#support contacts is not an actually multi-lingual field
						#but this way support can be added in the future
						if($contact->{'lang'}) {
							$multilangContact{'lang'} = $contact->{'lang'};
						} else {
							$multilangContact{'lang'} = 'en';
						}
						push(@contacts, \%multilangContact);
					}
					
					$data->{'prepared'}{$table} = \%contact;
					$data->{'prepared'}{$table.'_translation'}{'contacts'} = \@contacts;
				} elsif($namespace eq 'option') {
					my $style = $data->{$source};
					
					$style = 'substitution' if($style eq 'C');
					$style = 'postscript' if($style eq 'G');
					$style = 'pjl' if($style eq 'J');
					$style = 'composite' if($style eq 'X');
					$style = 'forced_composite' if($data->{'substyle'} eq 'F');
					
					$data->{'prepared'}{$table}{$destination} = $style
				}
			} elsif($mainGroup == 14) {
				next() if !defined($data->{$source});
				if($namespace eq 'driver') {#packages
					my %package;
					foreach my $package (@{$data->{$source}}) {
						$package{'driver_id'} = $data->{'name'};
						$package{'scope'} = $package->{'scope'};
						$package{'fingerprint'} = $package->{'fingerprint'}
							if($package->{'fingerprint'});
						$package{'name'} = $package->{'url'};
					}
					$data->{'prepared'}{$table} = \%package
				} elsif($namespace eq 'printer') {
					$data->{'prepared'}{$table}{$destination} = $data->{$source};
				}
			} elsif( defined($data->{$source})) {
				$data->{'prepared'}{$table}{$destination} = $data->{$source};
			}
		}
	}
}

#Insert data functions

sub getSortedKeys {
	my ($this, $data) = @_;
	
	my @keys;
	foreach my $key (keys %{$data}) {
		push(@keys, $key);
	}
	
	@keys = sort(@keys);
	
	#create a string that sums the contents of @keys, this allows us to identify 
	#similar sets of keys
	my $sumString;
	foreach my $key (@keys) {
		$sumString .= $key;
	}
	
	return (\@keys, $sumString);
}

sub getPreparedStatement {
	my ($this, $wantedTable, $data) = @_;

	my ($sortedKeys, $summedKeys) = $this->getSortedKeys($data);
	
	#use cached copy if present
	if (defined($this->{'cache'}{'preparedStatements'}{$wantedTable}{$summedKeys})) {
		return $this->{'cache'}{'preparedStatements'}{$wantedTable}{$summedKeys};
	}
	
	#Create the SQL
	my $statementBegining = "INSERT INTO ".$wantedTable."(";
	
	my $tableKeys = join(",", @{$sortedKeys});
	
	my $statementMiddle = ") VALUES (";
	
	my $bindValues;
	for(my $i = 0; $i < @{$sortedKeys}; $i++) {
		$bindValues .= ',' if ($i > 0);
		$bindValues .= '?';
	}
	
	my $statementEnd = ");";
	
	my $statementSql = $statementBegining . $tableKeys . 
	                      $statementMiddle . $bindValues . $statementEnd;
	
	my $sth = $this->{'dbh'}->prepare($statementSql);
	
	#add the sth to our cache
	$this->{'cache'}{'preparedStatements'}{$wantedTable}{$summedKeys} = $sth;
	
	return $sth;
}

sub insertData {
	my ($this, $data, $table) = @_;
	
	my $sth = $this->getPreparedStatement($table, $data);
	
	#Sort data alphabetically, required for prepared statements
	my ($sortedKeys) = $this->getSortedKeys($data);
	my @sortedData;
	foreach my $key (@{$sortedKeys}) {
		push(@sortedData, $data->{$key});
	}
	
	
	if( !$sth->execute(@sortedData) ) {
		print Dumper($data);
		die $this->{'dbh'}->errstr . "\n------\n";
	}
}

sub insertArrayData {
	my ($this, $data, $table, $key) = @_;
	foreach my $entry (@{ $data->{'prepared'}{$table}{$key} }) {
		$this->insertData($entry, $table);
	}
}

sub insertAssocs {
	my ($this, $data) = @_;

	my $select = "SELECT * FROM driver_printer_assoc WHERE driver_id = ? and printer_id = ?;";
	my $selectSTH = $this->{'dbh'}->prepare($select);
	my $delete = "DELETE FROM driver_printer_assoc WHERE driver_id = ? and printer_id = ?;";
	my $deleteSTH = $this->{'dbh'}->prepare($delete);
	
	#we get any prior data for this assoc and merge it with our new data
	#then delete the old record and insert our merged data
	foreach my $assoc (@{$data}) {
		$selectSTH->execute($assoc->{'driver_id'}, $assoc->{'printer_id'});
		
		#if a record for this assoc is present we merge the old data.
		#older data takes priority
		my $rowsEffected = 0;
		while ( my $row = $selectSTH->fetchrow_hashref() ) {
			foreach my $key (keys %{$row} ) {
				if($key eq 'color') {#colour can be zero
					$assoc->{$key} = $row->{$key} if defined($row->{$key});
				} else {
					$assoc->{$key} = $row->{$key} if $row->{$key};
				}
			}
			$rowsEffected++;
		}
		#by deleting the record we can reuse insertData() which does an insert
		#and not create duplicate records
		$deleteSTH->execute($assoc->{'driver_id'}, $assoc->{'printer_id'}) if ($rowsEffected);
		$this->insertData($assoc, 'driver_printer_assoc');
	}
}

#Xml type specific functions

sub pushOption {
	my ($this, $option) = @_;
	$this->setPreparedData($option, $this->{'optionPhonebook'});
	
	$this->insertData($option->{'prepared'}{'options'},'options');
	$this->insertArrayData($option, 'options_translation', 'comments');
	$this->insertArrayData($option, 'option_constraint', 'constraints');
	$this->insertArrayData($option, 'option_choice', 'vals');
	$this->insertArrayData($option, 'option_choice_translation', 'comments');
}

sub pushPrinter {
	my ($this, $printer) = @_;
	
	$this->setPreparedData($printer, $this->{'printerPhonebook'});
	
	$this->insertData($printer->{'prepared'}{'printer'},'printer');
	$this->insertArrayData($printer, 'printer_translation', 'comments');
	$this->insertArrayData($printer, 'margin', 'margins');
	$this->insertArrayData($printer, 'driver_printer_assoc', 'drivers');
	$this->insertAssocs($printer->{'prepared'}{'driver_printer_assoc'}{'drivers'});
	$this->insertArrayData($printer, 'driver_printer_assoc_translation', 'comments');
}

sub pushDriver {
	my ($this, $driver) = @_;
	
	$this->setPreparedData($driver, $this->{'driverPhonebook'});
	
	$this->insertData($driver->{'prepared'}{'driver'},'driver');
	$this->insertArrayData($driver, 'driver_translation', 'comments');
	if(%{ $driver->{'prepared'}{'driver_support_contact'} }) {
		$this->insertData($driver->{'prepared'}{'driver_support_contact'},'driver_support_contact');
		$this->insertArrayData($driver, 'driver_support_contact_translation', 'contacts');
	}
	if(%{ $driver->{'prepared'}{'driver_package'} }) {
		$this->insertData($driver->{'prepared'}{'driver_package'},'driver_package');
	}
	$this->insertArrayData($driver, 'margin', 'margins');
	$this->insertAssocs($driver->{'prepared'}{'driver_printer_assoc'}{'printers'});
	$this->insertArrayData($driver, 'driver_printer_assoc_translation', 'comments');
}

1;
