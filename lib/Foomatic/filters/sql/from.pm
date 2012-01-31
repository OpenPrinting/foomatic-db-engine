package Foomatic::filters::sql::from;
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

sub pullOption {
	my ($this, $optionId) = @_;
	
	my $perlOpt = Foomatic::filters::xml::xmlParse->defaultOptionData;
	
	my $querystr =
	    "SELECT * ".
	    "FROM options " .
	    "WHERE id=\"".$optionId."\"";
	my $sth = $this->{'dbh'}->prepare($querystr);
	$sth->execute();
	my $sqlOpt = $sth->fetchrow_hashref;
	
	foreach my $rule (@{$this->{'optionPhonebook'}}) {
		
		next if( !$rule->[3] 
			|| $rule->[3] ne 'options'
			|| !defined($rule->[4]) );
		
		
		my $group = $rule->[2];
		my $sqlKey = $rule->[4];
		my $perlKey = $rule->[1];
		next if (!defined($sqlOpt->{$sqlKey}));
		
		if(0 < $group && $group <= 10) {
			$perlOpt->{$perlKey} = $sqlOpt->{$sqlKey};

		} elsif ($group == 13) {
			my $val = $sqlOpt->{$sqlKey};
			$val = 'C' if($val eq 'substitution');
			$val = 'G' if($val eq 'postscript');
			$val = 'J' if($val eq 'pjl');
			$val = 'X' if($val eq 'composite');
			$val = 'F' if($val eq 'forced_composite');
						
			
			# forced_composite goes into the substyle key
			# while everything else goes into style.
			# In sql all are stored in the same field
			# so we have to sort out which one we're
			# working with.
			if ( ($perlKey eq 'substyle' && $val eq 'F') 
				|| ($perlKey ne 'substyle' && $val ne 'F') ) {
				$perlOpt->{$perlKey} = $val;
			}
			
		} else {
			next;
		}
	}
	
	return $perlOpt;
}

1;
