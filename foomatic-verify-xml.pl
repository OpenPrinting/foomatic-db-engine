use File::Basename;
use strict;
use warnings;

# This is foomatic-verify-xml, by default it verfies the entire XML DB against the xml schemes.
# It is a conveince wrapper for xmllint which you will need in your PATH.
# It takes two argument, the xmls to verify and the type of XML.
# If not passed we simply verify everything in the default path and get the types
# from the file path."


my $now = time;
my $defaultGlob = "/usr/share/foomatic/db/source/*/*.xml";
my $type = 0;
if ($#ARGV > -1) {
    if ($ARGV[0] eq "-h" || $ARGV[0] eq '--help') {
	print STDERR "
Usage: foomatic-verify-xml.pl [ \"glob\" , type]
	Assumes that the XML schemes are stored in a foomatic-db branch in a parent dir of our working dir
	
	glob: xmls to verify must be wrapped in quotes to avoid the shell expanding it, defaults to:
		\"/usr/share/foomatic/db/source/*/*.xml\"
	
	type: 'printer', 'driver', or 'option', defaults to being parsed from path.
		
Example: foomatic-verify-xml.pl '../foomatic-db/trunk/db/source/printer/*.xml' printer
";
	exit(1);
    } else {
		$defaultGlob = $ARGV[0];
		$type = $ARGV[1] if defined($ARGV[1]);
    }
}

print "Processing, any failures will be outputed.\n";
print "---------------\n";
print "failed xmls\n";
print "---------------\n";

my @files = glob($defaultGlob);
foreach my $file (@files) {
	my ($name,$path,$suffix) = fileparse($file);
	my $xmlType;
	
	if($type) {
		$xmlType = $type;
	} else {
		$path =~ m!/([^/]*)/$!;
		$xmlType = $1;
		$xmlType = $xmlType . 'ion' if ($xmlType eq 'opt');
	}
	
	my $line;
	$line = `xmllint --noout --schema '../foomatic-db/xmlschema/$xmlType.xsd'  '$file'  2>&1 `;
	#we have to pipe stderr to stdout
	
	if( $line =~ /fails/ ) {
		print $line;
	}
}

$now = time - $now;
# Print runtime #
printf("\n-------------\nTotal running time: %02d:%02d:%02d\n\n", int($now / 3600), int(($now % 3600) / 60), 
int($now % 60));
