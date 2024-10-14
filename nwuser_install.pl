#!/usr/bin/perl 

# - David Holland zzqzzq_zzq@hotmail.com - 7/7/03
# - David Holland david.w.holland@gmail.com 5/3/06

use strict; 

sub do_exec($$);
sub do_symlink($);
sub search_file($$);

use vars qw( $command );
use vars qw( $i $CRASHING $extraargs ); 
use vars qw( $dir $ndir );

use vars qw( $machine $is64bit $extra32flags ); 

use vars qw( $gcc32 $gcc64 $cflags $ldflags );

use vars qw( @sourceList   ); @sourceList = qw( attr dirs file nwuser mkdirp util io ); 
use vars qw( @object32List ); 
use vars qw( @object64List ); 
use vars qw( $sourceFile $object32File $object64File ); 

if( exists( $ENV{"CC32"} )) 	{ $gcc32 = $ENV{"CC32"}; 	} else { $gcc32 = "gcc -m32"; }
if( exists( $ENV{"CC64"} )) 	{ $gcc64 = $ENV{"CC64"}; 	} else { $gcc64 = "gcc"; }
if( exists( $ENV{"CFLAGS"} )) 	{ $cflags = $ENV{"CFLAGS"}; 	} else { $cflags = ""; }
if( exists( $ENV{"LDFLAGS"} ))	{ $ldflags = $ENV{"LDFLAGS"};	} else { $ldflags = ""; }

# Finish initializing the source/object lists.
for($i = 0; $i < scalar(@sourceList); $i++ ) { 
	$object32List[$i] = sprintf("%s.o", $sourceList[$i]); 
	$object64List[$i] = sprintf("%s64.o", $sourceList[$i]); 
	$sourceList[$i]   = sprintf("%s.c", $sourceList[$i]); 
}

foreach $dir ( ".", "nwuser" ) {
	if ( -f "$dir/nwuser.c" ) {
		$ndir = $dir;
		last;
	}
}

$CRASHING = 0; 
foreach $i (@ARGV) { 
	if( $i eq "crash" ) { 
		$CRASHING = 1; 
	} 
}

$extraargs = ""; 
if( $CRASHING ) { 
	$extraargs = $extraargs . " -DCRASH"; 
}

if( -f "/usr/include/attr/xattr.h" ) { 
	$extraargs = $extraargs .  " -DXATTR"; 
}

$machine = `/bin/uname -m`;
if( $machine =~ "x86_64" ) {
	# Extra places to look when building 32bit things on 64bit.
	$is64bit = 1; 
	$extra32flags = "-I/emul/ia32-linux/usr/include -L/emul/ia32-linux/usr/lib -L/emul/ia32-linux/lib -L/lib32 -L/usr/lib32";
} else {
	$is64bit = 0; 
	$extra32flags = "";
}

# (crudely) see if chdir() has nonnull attribute
if( search_file("/usr/include/unistd.h", "chdir.*nonnull")) {
	printf("NOTICE: NWUser: unistd.h/chdir() has nonnull attribute, setting -Wno-nonnull-compare\n"); 
	$cflags = sprintf("%s -Wno-nonnull-compare", $cflags); 
}

for( $i = 0; $i < scalar(@sourceList); $i++ ) { 
	$sourceFile = $sourceList[$i]; 
	$object32File = $object32List[$i];
	$object64File = $object64List[$i];

	if( $is64bit == 1 ) { 
		# Build 64bit
		$command = sprintf("%s %s -Wall -g -fPIC %s -o %s/%s -c %s/%s", $gcc64, $cflags, $extraargs, $ndir, $object64File, $ndir, $sourceFile); 
		do_exec(64, $command); 
	}
	# Build 32bit
	$command = sprintf("%s %s %s -Wall -g -fPIC %s -o %s/%s -c %s/%s", $gcc32, $cflags, $extra32flags, $extraargs, $ndir, $object32File, $ndir, $sourceFile); 
	do_exec(32, $command); 
}

# Linky, Linky.. 32 bit linky..
# $command = sprintf("%s %s %s -g -shared -Wl,-soname,nwuser.so -o %s/nwuser.so %s %s -ldl", $gcc32, $cflags, $x86_64, $ndir, join(" ", @objs), $ldflags); 

$command = sprintf("%s %s %s -g -shared",	$gcc32, $cflags, $extra32flags );
$command = sprintf("%s %s/%s",			$command, $ndir, join(" " . $ndir . "/", @object32List) ); 
$command = sprintf("%s -Wl,-soname,nwuser.so", 	$command ); 
$command = sprintf("%s -o %s/nwuser.so",	$command, $ndir );
$command = sprintf("%s %s -ldl", 		$command, $ldflags ); 
do_exec(32, $command); 

if( $is64bit == 1 ) { 
	# Linky, Linky.. 64 bit linky..
	# $command = sprintf("%s %s -g -shared -Wl,-soname,nwuser64.so -o %s/nwuser64.so %s %s -ldl", $gcc64, $cflags, $ndir, join(" ", @objs64), $ldflags); 
	$command = sprintf("%s %s -g -shared",		$gcc64, $cflags );
	$command = sprintf("%s %s/%s",			$command, $ndir, join(" " . $ndir . "/", @object64List) ); 
	$command = sprintf("%s -Wl,-soname,nwuser64.so", 	$command ); 
	$command = sprintf("%s -o %s/nwuser64.so",	$command, $ndir );
	$command = sprintf("%s %s -ldl", 		$command, $ldflags ); 
	do_exec(64, $command)
}

# If debugging, do not cleanup.
foreach $i (@ARGV) { 
	if( $i eq "debug" ) { 
		printf("NOTICE: Early exit for debugging...\n"); 
		exit(0);
	} 
}
printf("\n"); 

# Perform Cleanup.
for( $i = 0; $i < scalar(@object32List); $i++ ) { 
	$object32File = $object32List[$i];
	if( -f $ndir . "/" . $object32File) { 
		printf("NOTICE: Removing intermediate 32bit object file: %s/%s\n", $ndir, $object32File); 
		unlink($ndir . "/" . $object32File); 
	}
}

if( $is64bit == 1 ) { 
	printf("\n");
	for( $i = 0; $i < scalar(@object64List); $i++ ) { 
		$object64File = $object64List[$i];
		if( -f $ndir . "/" . $object64File) { 
			printf("NOTICE: Removing intermediate 64bit object file: %s/%s\n", $ndir, $object64File); 
			unlink($ndir . "/" . $object64File); 
		}
	}
	printf("\n");
}

# Generate appropriate symlinks. 
do_symlink($ndir);

printf("NOTICE: NWUser: Please check for errors above\n"); 
printf("NOTICE: NWUser: nwmovies executable built. Please modify your nwn startup command to\n"); 
printf("NOTICE: NWUser: set LD_PRELOAD to include 'nwuser.so', before executing nwmain.\n"); 

if( $is64bit == 1 ) { 
	printf("NOTICE: NWUser: set LD_PRELOAD to include 'nwuser64.so', as well, before executing nwmain.\n"); 
} 

exit(0); 

### Subs ###

sub do_symlink($) { 
	my ($ndir) = @_; 

# Install symlinks
	if( $ndir eq "." ) { 
		chdir(".."); 
	}

	printf("NOTICE: Installing nwuser.so symlink..."); 
	symlink("nwuser/nwuser.so", "nwuser.so"); 
	printf("\n"); 

	if( -f "nwuser/nwuser64.so" ) { 

		printf("NOTICE: Installing nwuser64.so symlink..."); 
		symlink("nwuser/nwuser64.so", "nwuser64.so"); 
		printf("\n"); 
	}

	return; 
}

sub do_exec($$) { 
	my( $bits, $cmd ) = @_; 
	my $ret; 

	printf("NOTICE: NWUser: Executing: %dbit: %s\n", $bits, $cmd); 
	$ret = system($cmd); 
	if ($ret == -1) { 
		die("ERROR: failed to execute: $!\n");
	} elsif ($ret & 127) {
		printf("ERROR: child died with signal %d, %s coredump\n", 
			($ret & 127),  ($ret & 128) ? 'with' : 'without');
		die("ERROR: Horrible screaming death in do_exec()\n"); 
	} elsif( ($ret >> 8) != 0 ) {
		printf("ERROR: child exited with value %d\n", $ret >> 8);
		die("ERROR: system() call failed.\n"); 
	}
	return; 
}

sub search_file($$) { 
	my($filename, $searchString) = @_; 
	my @dataArray;
	my $line; 
	my $ret;

#slurp...
        my $dataString = do {
                local $/ = undef;
                $ret = open( my $fh, "<", $filename );
                if( !$ret ) { 
                        printf("WARNING: Could not readfile: %s: %s\n", $filename, $! );
                        undef;
                }
                <$fh>;
        };

	@dataArray = split(/\n/, $dataString); 
	foreach $line (@dataArray) { 
		if( $line =~ /$searchString/ ) { 
			return(1); 
		}
	} 
	return(0); 
}

