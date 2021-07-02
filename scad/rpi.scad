


disp_w = 122;
disp_h = 79;
disp_th = 8;
disp_scr_w = 118;
disp_scr_h = 71;
disp_scr_offs = [2, 6, 0];

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
rpi_offs = [3, 0, -rpi_dz - 2];

module rpi_stl() {
	translate(rpi_offs + [rpi_w, disp_h, 0])
		rotate([0, 0, 180])
		import("rpi.stl");
}

module rpi() {
	rpi_stl();
	translate([0, disp_h - rpi_h, rpi_offs[2]])
		cube([rpi_w, rpi_h, rpi_th-1]);
	rpi_usb_hole();
	rpi_power_hole();
}

// ! rpi();

module rpi_usb_hole() {
	translate([-10, 22, -25])
		cube([15, 35, 17]);
}

module rpi_power_hole() {
	translate([68, 65, -27])
		cube([13, 33, 8.1]);
}

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
	hdmi_block();
}

// ! rpi_display();

enc_size = [132, 98, 30];
enc_offs = [-6, -4, -29];
wall_th = 1.5;
floor_th = 1;

enc_base_th = 10;
enc_top_th = 22;

module enclosure_top() {
	difference() {
		union() {
			translate([enc_offs[0], enc_offs[1], 0])
				cube([enc_size[0], enc_size[1], floor_th]);
			translate([-wall_th, -wall_th, -disp_th+1])
				cube([disp_w + 2*wall_th, disp_h + 2*wall_th, disp_th-1]);
			// /*
			translate(enc_offs + [0, 0, -enc_offs[2] - enc_top_th])
				cube([2*wall_th, enc_size[1], enc_top_th]);
			translate(enc_offs + [enc_size[0] - 2*wall_th, 0, -enc_offs[2] - enc_top_th])
				cube([2*wall_th, enc_size[1], enc_top_th]);
			translate(enc_offs + [0, 0, -enc_offs[2] - enc_top_th])
				cube([enc_size[0], 2*wall_th, enc_top_th]);
			translate(enc_offs + [0, enc_size[1] - 2*wall_th, -enc_offs[2] - enc_top_th])
				cube([enc_size[0], 2*wall_th, enc_top_th]);
			// */
		}

		rpi_display();
		rpi();
		enclosure_base();
	}
}

module enclosure_base() {
	difference() {
		union() {
			translate(enc_offs + [wall_th, wall_th, 0])
				cube([enc_size[0]-2*wall_th, 
					enc_size[1]-2*wall_th, floor_th]);
			translate(enc_offs + [wall_th, wall_th, 0])
				cube([wall_th, enc_size[1]-2*wall_th, enc_base_th]);
			translate(enc_offs + [enc_size[0] - 2*wall_th, wall_th, 0])
				cube([wall_th, enc_size[1]-2*wall_th, enc_base_th]);
			
			translate(enc_offs + [wall_th, wall_th, 0])
				cube([enc_size[0]-2*wall_th, wall_th, enc_base_th]);
			translate(enc_offs + [wall_th, enc_size[1] - 2*wall_th, 0])
				cube([enc_size[0]-2*wall_th, wall_th, enc_base_th]);
			for (dx=[0, disp_hole_c_c[0]])
				for (dy=[0, disp_hole_c_c[1]])
				if (dx != 0 || dy == 0)
				translate(disp_hole_offs + [dx, dy, enc_offs[2]])
				cylinder(r=5.5, h=5, $fn=32);
		}
		for (dx=[0, disp_hole_c_c[0]])
			for (dy=[0, disp_hole_c_c[1]])
			if (dx != 0 || dy == 0)
			translate(disp_hole_offs + [dx, dy, enc_offs[2]-1]) {
				cylinder(r=disp_thread_or, h=10, $fn=32);
				cylinder(r=4, h=3.5+1, $fn=32);
			}
		rpi_display();
		rpi();
		// # disp_holes();
		for (dx=[15/2:30:enc_size[0]]) 
			for (dy=[15/2:5*15:enc_size[1]]) 
			translate([dx, dy, -20])
			cylinder(r=3/2, h=30, center=true, $fn=32);
	}
}

// enclosure_top();
enclosure_base();
// %rpi_stl();
// % rpi_display();

module enclosure_base_short() {
	intersection() {
		# enclosure_base();
		translate([50, 50, -26])
			cube([200, 200, 1], center=true);
	}
}
// ! enclosure_base_short();


module enclosure_top_short() {
	intersection() {
		enclosure_top();
		translate([50, 50, -0])
			cube([200, 200, 1], center=true);
	}
}
// ! enclosure_top_short();
