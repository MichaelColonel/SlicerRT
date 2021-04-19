rotate([90, 0,0])
union() {
  color("#c09dbf")
  translate([0, 0, 135])
    cube([ 1000, 1800, 270], center=true);
  color("#5fa4df")
  translate([ 450, 0, 640+270])
    cube([ 100, 400, 1280], center=true);
  color("#5fa4df")
  translate([ -450, 0, 640+270])
    cube([ 100, 400, 1280], center=true);
  color("#5fa4df")
  translate([ 0, 800, 640+270])
    cube([ 400, 100, 1280], center=true);
}

