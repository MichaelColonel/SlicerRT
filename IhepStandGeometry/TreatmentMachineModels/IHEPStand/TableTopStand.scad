rotate([90, 0,0])
union() {
  color("#c09dbf")
  translate([0, 500+900, 135-1550.])
    cube([ 1000, 1800, 270], center=true);
  color("#5fa4df")
  translate([ 450, 500+900, 640+270-1550.])
    cube([ 100, 400, 1280], center=true);
  color("#5fa4df")
  translate([ -450, 500+900, 640+270-1550.])
    cube([ 100, 400, 1280], center=true);
  color("#5fa4df")
  translate([ 0, 2200, 640+270-1550.])
    cube([ 400, 100, 1280], center=true);
}
