--TEST--
\Win\Gdi\Color extended class
--SKIPIF--
<?php
	if (!extension_loaded('wingdi')) {
		die("skip wingdi extension not available\n");
	}
?>
--FILE--
<?php
use \Win\Gdi\Color;

class Shiny extends Color {
    public $red;

    public function __construct() {}
}
$class = new Shiny(655);
echo "$class\n";

unset($class->red);

$class->red = 255;
echo "$class\n";

var_dump($class);
?>
--EXPECTF--
#000000

Warning: main(): Property red cannot be unset in %s on line %d
#ff0000
object(Shiny)#%d (4) {
  ["red"]=>
  int(255)
  ["green"]=>
  int(0)
  ["blue"]=>
  int(0)
  ["hex"]=>
  int(255)
}