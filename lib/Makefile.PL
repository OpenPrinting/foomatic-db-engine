#! /usr/bin/perl

use ExtUtils::MakeMaker;

use Config;

my $rx = qr/^install(archlib|bin|man\ddir|privlib|script|sitearch|sitelib)/;

my %config = ( NAME => 'Foomatic',
	       AUTHOR => 'Grant Taylor <gtaylor+pht@picante.com>',
	       ABSTRACT => 'Foomatic printer configuration library code' );

$config{PREFIX} = "$ENV{PERLPREFIX}";
$config{PREFIX} = "$ENV{INSTALLPREFIX}$ENV{PERLPREFIX}" if $ENV{INSTALLPREFIX};

WriteMakefile( %config);
