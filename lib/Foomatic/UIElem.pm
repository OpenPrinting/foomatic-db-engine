
# TODO:
# sort by section and process that way
# order dep specific to particular options
# UI constraints
# various common non-UI options
# PJL options?

package Foomatic::UIElem;

# Call with name=>foo, label=>Foo, type=>Boolean or PickOne or PickMany

sub new {
    my ($type, %vals) = @_;
    return bless \%vals;
}

# default=>value
# order_real=>value
# order_section=>value
# order_keyword=>value

sub set {
    my ($this, $var, $val) = @_;
    $this->{$var} = $val;
}


sub add_option {
    my ($this, $option, $label, $ps) = @_;

    $ps =~ s!\r!!g;

    push (@{$this->{'options'}}, 
	  {'option' => $option,
	   'label' => $label,
	   'snippet' => $ps});
}


1;
