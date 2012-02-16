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
	
	#main data
	$perlOpt = $this->optionMain($perlOpt, $optionId);
	
	#choices
	$perlOpt = $this->optionChoices($perlOpt, $optionId);
	
	#multilang comments
	$perlOpt = $this->optionTranslations($perlOpt, $optionId);
	
	#constraints on the option
	$perlOpt = $this->optionConstraints($perlOpt, $optionId);
	

	
	return $perlOpt;
}

sub optionMain {
	my ($this, $perlOpt, $id) = @_;
	
	#Data from option table
	my $sth = $this->_query(
	    "SELECT * ".
	    "FROM options " .
	    "WHERE id=\"".$id."\"");
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

sub optionChoices {
	my ($this, $perlOpt, $id) = @_;
	
	#option_choice data
	my $sth = $this->_query(
	    "SELECT * ".
	    "FROM option_choice " .
	    "WHERE option_id=\"".$id."\"");
	my @choices = ();
	while (my $sqlOpt = $sth->fetchrow_hashref) {
		my %choice = ();
		$choice{'idx'} = $sqlOpt->{'id'};
		$choice{'comment'} = $sqlOpt->{'longname'} if $sqlOpt->{'longname'};
		$choice{'value'} = $sqlOpt->{'shortname'} if $sqlOpt->{'shortname'};
		$choice{'driverval'} = $sqlOpt->{'driverval'} if defined($sqlOpt->{'driverval'});
		push(@choices, \%choice);
		
		#get the comments,
		#This will overwrite the comment
		#we just got from the choice table
		%choice = %{ $this->optionTranslations(\%choice, $choice{'idx'}, $id) };
		
		#constraints on choice
		%choice = %{ $this->optionConstraints(\%choice, $id, $choice{'idx'}) };

	}
	
	#yup, they are called 'choices' and 'values'.
	#personally I think 'choices' is more specific.
	$perlOpt->{'vals'} = \@choices;
	
	return $perlOpt;
}

sub optionTranslations {
	my ($this, $perlOpt, $id, $secondId) = @_;
	#notice how 'comments' are also called 'translations',
	#'longnames'. 
	
	my $table = "options_translation";
	my $appendim = "";
	
	#Are we dealing with a option or choice translation
	if(defined($secondId)) {
		#must be a choice translation
		$table = "option_choice_translation";
		$appendim = "AND option_id = \"$secondId\""
	}
	
	# data
	my $sth = $this->_query(
	    "SELECT * ".
	    "FROM $table " .
	    "WHERE id=\"$id\" $appendim");
	my %translations = ();
	while (my $sqlOpt = $sth->fetchrow_hashref) {
		$translations{$sqlOpt->{'lang'}} = $sqlOpt->{'longname'};
	}
	
	#the multilang goes into 'comments'
	#legacy: english is stored in 'comment'
	$perlOpt->{'comments'} = \%translations;
	$perlOpt->{'comment'} = $translations{'en'} if defined($translations{'en'});
	
	
	return $perlOpt;
}

sub optionConstraints {
	my ($this, $perlOpt, $id, $choiceId) = @_;
	
	my $appendim = "";
	#Are we dealing with a option or choice constraint?
	if(defined($choiceId)) {
		#must be a choice constraint
		$appendim = "AND choice_id = \"$choiceId\""
	} else {
		$appendim = "AND is_choice_constraint = 0"
	}
	
	# data
	my $sth = $this->_query(
	    "SELECT * ".
	    "FROM option_constraint " .
	    "WHERE option_id=\"$id\" $appendim");
	my @constraints = ();
	while (my $sqlOpt = $sth->fetchrow_hashref) {
		my %constraint;
		$constraint{'printer'} = $sqlOpt->{'printer'} if ($sqlOpt->{'printer'});
		$constraint{'driver'} = $sqlOpt->{'driver'} if ($sqlOpt->{'driver'});
		$constraint{'arg_defval'} = $sqlOpt->{'defval'} if ($sqlOpt->{'defval'});
		
		#table uses 'true' and 'false' as strings
		$constraint{'sense'} = ($sqlOpt->{'sense'} eq 'true') ? 1 : 0;
		
		#the 'make' aka brand gets put into the printer field.
		#Yes this can be lossy. I did not write the original
		#sql schema so...
		#normally printer is formated as make-model.
		#for only make it will be make-
		if($constraint{'printer'} && $constraint{'printer'} =~ m/(.*)-$/) {
			$constraint{'make'} = $1;
			delete $constraint{'printer'};
		}
		
		push(@constraints, \%constraint);
	}
	
	#set and return
	$perlOpt->{'constraints'} = \@constraints if (@constraints);
	return $perlOpt;
}

# Returns a loaded statement handle for query
sub _query {
	my ($this, $query) = @_;
	my $sth = $this->{'dbh'}->prepare($query);
	if(!$sth) {
		print "\n\n$query\n\n";
	}
	$sth->execute();
	return $sth;
}

1;
