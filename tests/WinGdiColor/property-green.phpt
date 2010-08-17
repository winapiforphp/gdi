--TEST--
\Win\Gdi\Color->$green property
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

$class->green = 255;
echo "$class\n";

$class->green = -3;
echo "$class\n";

$class->green = 855;
echo "$class\n";

$class->hex = 'FFF';
echo "$class->green\n";

unset($class->green);

var_dump($class);
?>
--EXPECTF--
#000000
#0000ff
#000000
#0000ff
255

Warning: main(): Property green cannot be unset in %s on line %d
object(Win\Gdi\Color)#%d (4) {
  ["red"]=>
  int(255)
  ["green"]=>
  int(255)
  ["blue"]=>
  int(255)
  ["hex"]=>
  int(16777215)
}