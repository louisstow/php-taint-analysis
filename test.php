<?php

function inc ($a) {
	$tmp = 3;
	if ($tmp + $a > 4) {
		$tmp *= 3;
	}
	$tmp += $a;

	return $tmp;
}

$b = 9;
$b = inc($b);
?>
