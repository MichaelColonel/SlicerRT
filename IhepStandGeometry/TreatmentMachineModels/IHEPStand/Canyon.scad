// Gray cement color
color("#9ea4a7")
rotate([90, 0,0])
union() {

  // floor
  difference()
  {
    translate([ -980, -52.5, -300.-1550.])
      cube([ 7000, 4895, 600.], center=true);
    translate([ 0, 0, -300.-1550.])
      cylinder( h=601, d=3010., center=true);
  }

  // beam front wall
  difference()
  {
    translate([ 3020, -52.5, 1900.0-1550.])
      cube([ 1000, 4895, 5000], center=true);
    translate([ 3020, 0., 1550.-1550.])
      cube([ 1001, 450., 450.], center=true);
  }

  // back stand wall
  translate([ -980, 5895/2 - 52.5, 1900-1550.])
    cube([ 9000, 1000., 5000.], center=true);

  // left wall
  translate([ -4980, -52.5, 1900.0-1550.])
    cube([ 1000, 4895, 5000], center=true);
}

color("#676767")
rotate([90, 0,0])
union() {
  translate([0, 1050, 900+12.5-1550.])
    cube([ 530, 2000, 25], center=true);
  translate([0, 1050, 900+12.5-33-1550.])
    cube([ 400, 2000, 66], center=true);
}
