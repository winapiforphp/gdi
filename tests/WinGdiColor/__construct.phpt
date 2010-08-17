--TEST--
\Win\Gdi\Color->__construct method
--SKIPIF--
<?php
	if (!extension_loaded('wingdi')) {
		die("skip wingdi extension not available\n");
	}
?>
--FILE--
<?php
use \Win\Gdi\Color;

$class = new Color();
echo "$class\n";

$class = new Color(255);
echo "$class\n";

$class = new Color(255, 255);
echo "$class\n";

$class = new Color(255, 255, 255);
echo "$class\n";

$class = new Color('CCC');
echo "$class\n";

$class = new Color('#FFF');
echo "$class\n";

$class = new Color('FFFFFF');
echo "$class\n";

$class = new Color('#CC0000');
echo "$class\n";

$class = new Color(469, 569, 815);
echo "$class\n";

$class = new Color(-469, -8, -34);
echo "$class\n";

try {
	$class = new Color('f');
} catch (Exception $e) {
	echo $e->getMessage(), PHP_EOL;
}

try {
	$class = new Color('foobb');
} catch (Exception $e) {
	echo $e->getMessage(), PHP_EOL;
}

try {
	$class = new Color(array());
} catch (Exception $e) {
	echo $e->getMessage(), PHP_EOL;
}

try {
	$class = new Color(5, 9, 10, 12, 15, 22);
} catch (Exception $e) {
	echo $e->getMessage(), PHP_EOL;
}
?>
--EXPECT--
#000000
#ff0000
#ff00ff
#ffffff
#ffffff
#ffffff
#ffffff
#cc0000
#ffffff
#000000
Invalid color string
Invalid color string
Invalid color, must be string or integer
Win\Gdi\Color::__construct() expects at most 3 parameters, 6 given