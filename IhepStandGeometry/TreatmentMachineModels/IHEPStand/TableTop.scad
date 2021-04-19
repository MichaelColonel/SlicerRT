rotate([90, 0,0])
color("#676767")
union() {
  translate([0, -400, -12.5])
    cube([ 530, 2000, 25], center=true);
  translate([0, -400, -12.5-33])
    cube([ 400, 2000, 66], center=true);
}