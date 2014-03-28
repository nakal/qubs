#!/usr/bin/perl -w

use strict;

my $x;
my $y;

if (!defined($ARGV[0])) {
	print "Syntax: nqueens.pl number-of-queens\n";
	exit(1);
}

my $n = int($ARGV[0]);
if ($n<1 || $n>1000) {
	print "number-of-queens = 1 ... 1000\n";
	exit(1);
}

print "# ******** At least 1 queen a row\n";
for (my $iy=1; $iy<$n; $iy++) { print "AND\n"; }
for (my $iy=1; $iy<=$n; $iy++) {
	print "# Check for 1 queen in row for row $iy\n";
	for (my $ix=1; $ix<$n; $ix++) { print "OR\n"; }
	for (my $ix=1; $ix<=$n; $ix++) {
		for (my $iix=1; $iix<$n+3; $iix++) { print "AND\n"; }
		for (my $iix=1; $iix<=$n; $iix++) {
			print "NOT\n" if ($iix!=$ix);
			print "x_" . $iix . "_" . $iy . "\n";
		}
		&q_horiz($ix, $iy, $n);
		&q_vert($ix, $iy, $n);
		&q_diag($ix, $iy, $n);
	}
}

# print "# Queen rules\n";
# for ($x=1; $x<$n; $x++) { print "OR\n"; }
# for ($x=1; $x<=$n; $x++) {
# 	for ($y=1; $y<$n; $y++) { print "OR\n"; }
# 
# 	for ($y=1; $y<=$n; $y++) {
# 		print "AND\nAND\n";
# 		&q_horiz($x,$y,$n);
# 		&q_vert($x,$y,$n);
# 		&q_diag($x,$y,$n);
# 	}
# }

sub q_horiz {
	my ($x, $y, $n) = @_;

	print "# horizontal check for ($x,$y)\n";
	for (my $i=1; $i<$n; $i++) { print "AND\n"; }

	for (my $i=1; $i<=$n; $i++) {
		if ($i!=$x) {
			print "NOT\n";
		}

		print "x_" . $i . "_" . $y . "\n";
	}
}

sub q_vert {
	my ($x, $y, $n) = @_;

	print "# vertical check for ($x,$y)\n";
	for (my $i=1; $i<$n; $i++) { print "AND\n"; }

	for (my $i=1; $i<=$n; $i++) {
		if ($i!=$y) {
			print "NOT\n";
		}

		print "x_" . $x . "_" . $i . "\n";
	}
}

sub q_diag {
	my ($x, $y, $n) = @_;

	print "# diagonal check for ($x,$y)\n";

	my @out = ();

	my $px = $x-1;
	my $py = $y-1;

	while ($px>=1 && $py>=1) {
		push @out, "x_" . $px . "_" . $py;
		$px--; $py--;
	}

	$px = $x+1;
	$py = $y+1;

	while ($px<=$n && $py<=$n) {
		push @out, "x_" . $px . "_" . $py;
		$px++; $py++;
	}

	$px = $x-1;
	$py = $y+1;

	while ($px>=1 && $py<=$n) {
		push @out, "x_" . $px . "_" . $py;
		$px--; $py++;
	}

	$px = $x+1;
	$py = $y-1;

	while ($px<=$n && $py>=1) {
		push @out, "x_" . $px . "_" . $py;
		$px++; $py--;
	}

	for (my $i=0; $i<scalar(@out); $i++) { print "AND\n"; }
	print "x_" . $x . "_" . $y . "\n";
	foreach (@out) {
		print "NOT\n";
		print $_ . "\n";
	}
}

exit(0);
