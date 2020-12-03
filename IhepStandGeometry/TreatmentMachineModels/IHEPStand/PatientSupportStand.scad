union() {
  color("#c09dbf")
  translate([0, 50+90, 13.5])
    cube([ 100, 180, 27], center=true);
  color("#5fa4df")
  translate([ 45, 50+90, 64+27])
    cube([ 10, 40, 128], center=true);
  color("#5fa4df")
  translate([ -45, 50+90, 64+27])
    cube([ 10, 40, 128], center=true);
  color("#5fa4df")
  translate([ 0, 220, 64+27])
    cube([ 40, 10, 128], center=true);
}
