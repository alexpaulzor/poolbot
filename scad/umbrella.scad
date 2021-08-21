use <sprockets.scad>;
use <drive_motor.scad>;

module drive_sprocket() {
    translate([0, 0, -10])
        sprocket(size=40, teeth=6, bore=5/16, hub_diameter=mm2inches(15), hub_height=mm2inches(10));
}

// ! drive_sprocket();

crank_nub_w = 8;
crank_nub_d = 12; // TODO: measure

module crank_nub() {
    // TODO
    translate([0, 0, crank_nub_d/2])
        cube([crank_nub_w, crank_nub_w, 
            crank_nub_d], center=true);
}

// !crank_nub();

module crank_adapter() {
    difference() {
        union() {
            crank_nub();
            drive_sprocket();
        }
        // # motor_shaft();
        
    }
    * %drive_motor();
}

crank_adapter();
