
# This is a modified version of libxml-enno's grove::path, which is
# less prone to blowing up.

package XML::Grove::Path;

use XML::Grove;
use XML::Grove::XPointer;
use UNIVERSAL;

sub at_path {
    my $element = shift;	# or Grove
    my $path = shift;

    $path =~ s|^/*||;

    my @path = split('/', $path);

    return (_at_path ($element, [@path]));
}

sub _at_path {
    my $element = shift;	# or Grove
    my $path = shift;
    my $segment = shift @$path;

    # segment := [ type ] [ '[' index ']' ]
    #
    # strip off the first segment, finding the type and index
    $segment =~ m|^
                ([^\[]+)?     # - look for an optional type
                              #   by matching anything but '['
                (?:           # - don't backreference the literals
                  \[          # - literal '['
                    ([^\]]+)  # - index, any non-']' chars
                  \]          # - literal ']'
                )?            # - the whole index is optional
               |x;
    my ($node_type, $instance, $match) = ($1, $2, $&);
    # issues:
    #   - should assert that no chars come after index and before next
    #     segment or the end of the query string

    $instance = 1 if !defined $instance;

    my $object = $element->xp_child ($instance, $node_type);

    if ($#$path eq -1) {
        return $object;
    } elsif (!defined($object)) {
	return undef;
    } elsif (!$object->isa('XML::Grove::Element')) {
        # FIXME a location would be nice.
        die "\`$match' doesn't exist or is not an element\n";
    } else {
        return (_at_path($object, $path));
    }
}

package XML::Grove::Document;

sub at_path {
    goto &XML::Grove::Path::at_path;
}

package XML::Grove::Element;

sub at_path {
    goto &XML::Grove::Path::at_path;
}

1;
