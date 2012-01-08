package Foomatic::filters::sql::sqlLayer;

use strict;
use warnings;
use Data::Dumper;

use DBI;
use Foomatic::filters::phonebook;


sub new( $ $ $ ) {
	my ($class, $dbHandle, $dbType, $version) = @_;
	my $this = {};
	bless $this, $class;
	
	
	$this->{'dbh'} = $dbHandle;
	
	# By default sqlite fsync's after every write, this is extremly slow
	$this->{'dbh'}->do('PRAGMA synchronous = OFF;') if ($dbType eq 'sqlite');
	
	$this->{'type'} = $dbType;
	
	
	#default to 0, perfect compatablity with C primary xml parsing
	# 1 = compatibility of C combo parsing
	# 2 = multilingual support and printers_byname in drivers
	$this->{'version'} = 0;
	$this->{'version'} = $version if(defined($version));
	
	my $phonebook = Foomatic::filters::phonebook->new($this->{'version'});
	$this->{'printerPhonebook'} = $phonebook->printer();
	$this->{'driverPhonebook'}  = $phonebook->driver();
	$this->{'optionPhonebook'}  = $phonebook->option();
	$this->{'schemaPhonebook'}  = $phonebook->schema();
	
	
	return $this;
}

#Schema fuctions
sub getSchema {
	my ($this, $table) = @_;
	
	return $this->{'schemaPhonebook'}{$table};
}

sub initDatabase {
	my ($this) = @_;
	
	#Create the Tables
	foreach my $table (keys %{$this->{'schemaPhonebook'}}) {
		my $schema = $this->getSchema($table);
		my $sth = $this->{'dbh'}->prepare($schema);
		
		$sth->execute();
	}
	$this->{'dbh'}->commit();
}

1;
