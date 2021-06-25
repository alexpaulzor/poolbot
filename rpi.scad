


disp_w = 122;
disp_h = 78;
disp_th = 8;
disp_scr_w = disp_w - 8;
disp_scr_h = disp_h - 11;
disp_scr_offs = [4, 4, 0];

disp_hole_c_c = [93, 69];
disp_hole_offs = [24, 5, 0];

disp_standoff_h = 16;
disp_standoff_or = 3/2;
disp_thread_or = 2/2;
disp_thread_h = 3;

rpi_w = 85;
rpi_h = 56;
rpi_th = 17;
rpi_dz = 25;
rpi_offs = [3, 1, -rpi_dz - 2];
module rpi() {
	% translate(rpi_offs + [rpi_w, disp_h, 0])
		rotate([0, 0, 180])
		import("rpi.stl");
	translate([0, disp_h - rpi_h, rpi_offs[2]])
		cube([rpi_w, rpi_h, rpi_th]);
	translate([-5, 23, -25])
		cube([10, 33, 17]);
	translate([69, 65, -27])
		cube([12, 33, 8]);
}

// ! rpi();

hdmi_block_size = [18, 12, 15];
hdmi_block_offs = [44, disp_h, -disp_th - hdmi_block_size[2] - 1];

module hdmi_block() {
	translate(hdmi_block_offs)
		cube(hdmi_block_size);
}

module rpi_screen() {
	color(alpha=0.5)
		translate(disp_scr_offs)
		cube([disp_scr_w, disp_scr_h, 4]);
}

module disp_standoffs() {
	for (dx=[0, disp_hole_c_c[0]])
		for (dy=[0, disp_hole_c_c[1]])
		translate(disp_hole_offs + [dx, dy]) {
			translate([0, 0, -disp_standoff_h - disp_th])
				cylinder(
					r=disp_standoff_or, 
					h=disp_standoff_h, $fn=32);
				translate([0, 0, -disp_standoff_h - disp_th - disp_thread_h])
				cylinder(
					r=disp_thread_or, 
					h=disp_thread_h, $fn=32);
		}
}

module disp_holes() {
	for (dx=[0, disp_hole_c_c[0]])
		for (dy=[0, disp_hole_c_c[1]])
		translate(disp_hole_offs + [dx, dy]) {
			// translate([0, 0, -disp_standoff_h - disp_th])
			// 	cylinder(
			// 		r=disp_standoff_or, 
			// 		h=disp_standoff_h, $fn=32);
			translate([0, 0, -disp_standoff_h - disp_th - disp_thread_h])
				cylinder(
					r=disp_thread_or, 
					h=disp_standoff_h, $fn=32);
		}
}

module rpi_display() {
	translate([0, 0, -disp_th]) {
		cube([disp_w, disp_h, disp_th]);	
	}
	rpi_screen();
	disp_standoffs();
	rpi();
	hdmi_block();
}

// rpi_display();

enc_size = [130, 95, 28];
enc_offs = [-4, -2, -28];
wall_th = 1;

module enclosure_top() {
	difference() {
		union() {
			translate([enc_offs[0], enc_offs[1], 0])
				cube([enc_size[0], enc_size[1], wall_th]);
			translate([-wall_th, -wall_th, -4])
				cube([disp_w + 2*wall_th, disp_h + 2*wall_th, 4]);
			// /*
			translate(enc_offs)
				cube([2*wall_th, enc_size[1], enc_size[2]]);
			translate(enc_offs + [enc_size[0] - 2*wall_th, 0, 0])
				cube([2*wall_th, enc_size[1], enc_size[2]]);
			translate(enc_offs)
				cube([enc_size[0], 2*wall_th, enc_size[2]]);
			translate(enc_offs + [0, enc_size[1] - 2*wall_th, 0])
				cube([enc_size[0], 2*wall_th, enc_size[2]]);
			// */
		}

		# rpi_display();
		enclosure_base();
	}
}

module enclosure_base() {
	difference() {
		union() {
			translate(enc_offs + [wall_th, wall_th, 0])
				cube([enc_size[0]-2*wall_th, 
					enc_size[1]-2*wall_th, wall_th]);
			translate(enc_offs + [wall_th, wall_th, 0])
				cube([wall_th, enc_size[1]-2*wall_th, enc_size[2]/2]);
			translate(enc_offs + [enc_size[0] - 2*wall_th, wall_th, 0])
				cube([wall_th, enc_size[1]-2*wall_th, enc_size[2]/2]);
			
			translate(enc_offs + [wall_th, wall_th, 0])
				cube([enc_size[0]-2*wall_th, wall_th, enc_size[2]/2]);
			translate(enc_offs + [wall_th, enc_size[1] - 2*wall_th, 0])
				cube([enc_size[0]-2*wall_th, wall_th, enc_size[2]/2]);
			for (dx=[0, disp_hole_c_c[0]])
				for (dy=[0, disp_hole_c_c[1]])
				translate(disp_hole_offs + [dx, dy, enc_offs[2]])
				cylinder(r=4, h=4);
		}
		for (dx=[0, disp_hole_c_c[0]])
			for (dy=[0, disp_hole_c_c[1]])
			translate(disp_hole_offs + [dx, dy, enc_offs[2] - 1]) {
				cylinder(r=disp_thread_or, h=6);
				cylinder(r=3, h=4);
			}
		rpi_display();
		// # disp_holes();
	}
}

enclosure_top();
// enclosure_base();

module enclosure_base_short() {
	intersection() {
		enclosure_base();
		# translate([50, 50, -25.5])
			cube([200, 200, 2], center=true);
	}
}
// enclosure_base_short();