--TEST--
\Win\Gdi\Color->__toString method
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

$class = new Color(255, 255, 255);
echo strtoupper($class), PHP_EOL;

$class = new Color('FFF');
echo $class . "foobar\n";

$class = new Color('#CC0000');
echo $class->__toString();

?>
--EXPECT--
#000000
#FFFFFF
#fffffffoobar
#cc0000