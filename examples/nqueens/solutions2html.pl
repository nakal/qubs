#!/usr/bin/perl -w

use strict;

print "<html>
<head><title>Chessboard</title></head>
<body>\n";

my $found = 0;
while (<STDIN>) {
	if (m/^Variable order:/) {
		$found=1;
		last;
	}
}
if (!$found) {
	die "No solutions found in this file.\n";
}

my @vars = split /,/,<STDIN>;
my $solutionnr = 1;
my $numvars = scalar(@vars);
my $checkboardsize = int(sqrt($numvars));

printf "<h1>Solutions for the $checkboardsize Queens Puzzle</h1>\n";

while (<STDIN>) {
	&output_solution($solutionnr++, $_);
}

print "</body>
</html>\n";

exit(0);

sub output_solution {

	my ($nr, $solution) = @_;

	my %solh = ();
	chomp $solution;
	my @sola = split / /,$solution;
	if (scalar(@sola)!=$numvars) { die "Illegal input!\n$solution"; }

	my $control = 0;
	foreach (my $i=0; $i<$numvars; $i++) {
		my $r = $sola[$i];
		if ($r eq "1") {
			$solh{$vars[$i]}=1;
			$control++;
		}
	}
	if ($control!=$checkboardsize) { die "Illegal input ($control queens in $numvars variables)!\n$solution"; }

	printf "<p>Solution number %u:</p>", $nr;
	print "<table style=\"border-collapse: collapse; font-family: Verdana,Mono,Sans,Serif;\">\n";
	print "<tr><th>&nbsp;</th>";
	for (my $i=0; $i<$checkboardsize; $i++) {
		printf "<th>%c</th>", (65+$i);
	}
	print "</tr>\n";

	for (my $y=$checkboardsize; $y>=1; $y--) {
		printf "<tr><th>%d</th>", $y;
		my $bgcolor = $y % 2;
		for (my $x=1; $x<=$checkboardsize; $x++) {
			printf "<td style=\"background-color: %s; color: %s\">%s</td>", ($bgcolor ? "black" : "white"), ($bgcolor ? "white" : "black"), defined($solh{"x_" . $x . "_" . $y}) ? "&#x2655;" : "&nbsp;";
			$bgcolor = 1 - $bgcolor;
		}
		print "</tr>\n";
	}
	print "</table><br />\n";
}

