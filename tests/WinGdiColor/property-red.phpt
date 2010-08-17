--TEST--
\Win\Gdi\Color->$red property
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

$class->red = 255;
echo "$class\n";

$class->red = -51;
echo "$class\n";

$class->red = 546;
echo "$class\n";

$class->hex = 'FFF';
echo "$class->red\n";

unset($class->red);

var_dump($class);
?>
--EXPECTF--
#000000
#ff0000
#000000
#ff0000
255

Warning: main(): Property red cannot be unset in %s on line %d
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