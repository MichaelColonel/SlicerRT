// Gray cement color
color("#9ea4a7")
union() {

  // floor
  difference()
  {
    translate([ -98, -5.25, -30.])
      cube([ 700.0, 489.5, 60.], center=true);
    translate([ 0, 0, -30.])
      cylinder( h=60.1, d=301., center=true);
  }

  // beam front wall
  difference()
  {
    translate([ 302, -5.25, 190.00])
      cube([ 100.0, 489.5, 500], center=true);
    translate([ 302, 0., 155.])
      cube([ 100.1, 45., 45.], center=true);
  }

  // back stand wall
  translate([ -98, 589.5/2 - 5.25, 190])
    cube([ 900.0, 100., 500.], center=true);

  // left wall
  translate([ -498, -5.25, 190.00])
    cube([ 100.0, 489.5, 500], center=true);
}