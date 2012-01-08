package Foomatic::util;

use Exporter 'import';
@EXPORT = qw(getCleanId);

sub getCleanId {
	#remove everything before the leading slash
	my ($id) = @_;
	$id =~ s/^[^\/]*\///;
	return $id;
}

1;
