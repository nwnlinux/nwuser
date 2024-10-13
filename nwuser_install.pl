#!/usr/bin/perl 

# - David Holland zzqzzq_zzq@hotmail.com - 7/7/03
# - David Holland david.w.holland@gmail.com 5/3/06

use strict; 

use vars qw( $command );
use vars qw( $file $obj ); 
use vars qw( @files ); 
use vars qw( @objs @objs64 ); 
use vars qw( $i $CRASHING $extraargs ); 

use vars qw( $dir $ndir );

use vars qw( $machine $x86_64 ); 

use vars qw( $gcc32 $gcc64 $cflags $ldflags );

@files = qw( attr.c dirs.c file.c nwuser.c mkdirp.c util.c io.c ); 

$CRASHING = 0; 

foreach $i (@ARGV) { 
	if( $i eq "crash" ) { 
		$CRASHING = 1; 
	} 
}

foreach $dir ( ".", "nwuser" ) {
	if ( -f "$dir/nwuser.c" ) {
		$ndir = $dir;
		last;
	}
}

$extraargs = ""; 
if( $CRASHING ) { 
	$extraargs = $extraargs . " -DCRASH"; 
}

if( -f "/usr/include/attr/xattr.h" ) { 
	$extraargs = $extraargs .  " -DXATTR"; 
}

# Yes, the usage of gcc32 vs gcc64 seems backwards, but well.. ummm
# I dunno.. its just backwards as $gcc64 == gcc "native bit size" 
# and I'm sure I'll never remember this 5 minutes from now.

if( exists( $ENV{"CC32"} )) {
        $gcc32 = $ENV{"CC32"};
} else {
        $gcc32 = "gcc -m32";
}

if( exists( $ENV{"CC64"} )) {
        $gcc64 = $ENV{"CC64"};
} else {
        $gcc64 = "gcc";
}

if( exists( $ENV{"CFLAGS"} )) {
        $cflags = $ENV{"CFLAGS"};
} else {
        $cflags = "";
}

if( exists( $ENV{"LDFLAGS"} )) {
        $ldflags = $ENV{"LDFLAGS"};
} else {
        $ldflags = "";
}


$machine = `/bin/uname -m`;

if( $machine =~ "x86_64" ) {
	$x86_64 = "-I/emul/ia32-linux/usr/include -L/emul/ia32-linux/usr/lib -L/emul/ia32-linux/lib -L/lib32 -L/usr/lib32";
} else {
	$x86_64 = "";
}

foreach $file ( @files ) {
	$obj = $file; 
	chop($obj); chop($obj);   # Remove '.c'
	push( @objs, $ndir . "/" . $obj . ".o" ); 
	if( $x86_64 ne "" ) { 
		push( @objs64, $ndir . "/" . $obj . "64.o" ); 

		$command = sprintf("%s %s -Wall -g -fPIC %s -o %s/%s64.o -c %s/%s", $gcc64, $cflags, $extraargs, $ndir, $obj, $ndir, $file); 
		printf("NOTICE: Executing: %s\n", $command); 
		system($command); 
	}

	$command = sprintf("%s %s %s -Wall -g -fPIC %s -o %s/%s.o -c %s/%s", $gcc32, $cflags, $x86_64, $extraargs, $ndir, $obj, $ndir, $file); 
	printf("NOTICE: Executing: %s\n", $command); 
	system($command); 
}

$command = sprintf("%s %s %s -g -shared -Wl,-soname,nwuser.so -o %s/nwuser.so %s %s -ldl", $gcc32, $cflags, $x86_64, $ndir, join(" ", @objs), $ldflags); 
printf("NOTICE: NWUser: Executing: %s\n", $command); 
system($command); 

if( $x86_64 ne "" ) { 
	$command = sprintf("%s %s -g -shared -Wl,-soname,nwuser64.so -o %s/nwuser64.so %s %s -ldl", $gcc64, $cflags, $ndir, join(" ", @objs64), $ldflags); 
	printf("NOTICE: NWUser: Executing: %s\n", $command); 
	system($command); 
}

# Generate appropriate symlinks. 
if( $ndir eq "." ) { 
	chdir(".."); 
}
symlink( "nwuser/nwuser.so", "nwuser.so" ); 
if( $x86_64 ne "" ) { 
	symlink( "nwuser/nwuser64.so", "nwuser64.so" ); 
}

printf("NOTICE: NWUser: Please check for errors above\n"); 
printf("NOTICE: NWUser: nwmovies executable built. Please modify your nwn startup command to\n"); 
printf("NOTICE: NWUser: set LD_PRELOAD to include 'nwuser.so', before executing nwmain.\n"); 

if( $x86_64 ne "" ) { 
	printf("NOTICE: NWUser: set LD_PRELOAD to include 'nwuser64.so', as well, before executing nwmain.\n"); 
} 

exit(0); 
