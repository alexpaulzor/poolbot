
IN_MM = 25.4;

min_th = 1.5;

button_thread_or = 8/2;
button_h = 21;
button_base_or = 10/2;
button_base_h = 9;
button_thread_h = 7;
button_wire_h = 13;
button_or = 5/2;
button_cap_h = 3;
button_cap_or = 6/2;

module button(colorname="yellow") {
	translate([0, 0, -button_base_h]) {
		cylinder(r=button_or, h=button_h - button_cap_h);
		color("black")
			cylinder(r=button_base_or, h=button_base_h);
		color("silver")
			translate([0, 0, button_base_h])
			cylinder(r=button_thread_or, h=button_thread_h, $fn=64);
		translate([0, 0, button_h - button_cap_h])
			color(colorname)
			cylinder(r=button_cap_or, h=button_cap_h);
		color("black", 0.6)
			translate([0, 0, -button_wire_h])
			cylinder(r=button_thread_or, h=button_wire_h);
	}
}

// ! button();

label_h = 2 * min_th;

module mode_button_labels() {
	translate([0, 30, 0])
		linear_extrude(label_h)
		text(
			"MODE", 
			font="Helvetica:style=Negreta", 
			size=9, 
			halign="center", 
			valign="center");
	translate([5, 15, 0])
		linear_extrude(label_h)
		text(
			"SPA", 
			font="Helvetica:style=Negreta", 
			size=7, 
			halign="right", 
			valign="center");
	translate([5, 0, 0])
		linear_extrude(label_h)
		text(
			"SPILL", 
			font="Helvetica:style=Negreta", 
			size=7, 
			halign="right", 
			valign="center");
	translate([5, -15, 0])
		linear_extrude(label_h)
		text(
			"POOL", 
			font="Helvetica:style=Negreta", 
			size=7, 
			halign="right", 
			valign="center");
	translate([5, -30, 0])
		linear_extrude(label_h)
		text(
			"CLEAN", 
			font="Helvetica:style=Negreta", 
			size=7, 
			halign="right", 
			valign="center");
}

module mode_buttons() {
	translate([12, 15, 0])
		button("red");
	translate([12, 0, 0])
		button("green");
	translate([12, -15, 0])
		button("blue");
	translate([12, -30, 0])
		button("yellow");
	
}

button_mode_panel_size = [60, 100, min_th];

module mode_button_panel() {
	difference() {
		translate([0, 0*-7.5, min_th/2])
			cube(button_mode_panel_size, center=true);
		mode_buttons();
		mode_button_labels();
		for (x=[-1, 1])
			for (y=[-1, 1])
				translate([
					x*(button_mode_panel_size[0]/2 - 15/2),
					y*(button_mode_panel_size[1]/2 - 15/2) - 0*7.5,
					-1])
				cylinder(r=3/2, h=5, $fn=32);
	}
	% mode_buttons();
	translate([-1, 1, 0])
		mode_button_labels();
}

// translate([-100, 0, 0])
// mode_button_panel();


module speed_button_labels() {
	translate([0, 45, 0])
		linear_extrude(label_h)
		text(
			"SPEED", 
			font="Helvetica:style=Negreta", 
			size=9, 
			halign="center", 
			valign="center");
	translate([-5, 30, 0])
		linear_extrude(label_h)
		text(
			"MAX", 
			font="Helvetica:style=Negreta", 
			size=7, 
			halign="left", 
			valign="center");
	translate([-5, 15, 0])
		linear_extrude(label_h)
		text(
			"HI", 
			font="Helvetica:style=Negreta", 
			size=7, 
			halign="left", 
			valign="center");
	translate([-5, 0, 0])
		linear_extrude(label_h)
		text(
			"LOW", 
			font="Helvetica:style=Negreta", 
			size=7, 
			halign="left", 
			valign="center");
	translate([-5, -15, 0])
		linear_extrude(label_h)
		text(
			"MIN", 
			font="Helvetica:style=Negreta", 
			size=7, 
			halign="left", 
			valign="center");
	translate([-5, -30, 0])
		linear_extrude(label_h)
		text(
			"OFF", 
			font="Helvetica:style=Negreta", 
			size=7, 
			halign="left", 
			valign="center");
}

module speed_buttons() {
	translate([-12, 30, 0])
		button("red");
	translate([-12, 15, 0])
		button("yellow");
	translate([-12, 0, 0])
		button("blue");
	translate([-12, -15, 0])
		button("green");
	translate([-12, -30, 0])
		button("black");
	
}

speed_button_panel_size = [60, 100, min_th];

module speed_button_panel() {
	difference() {
		translate([0, 7.5, min_th/2])
			cube(speed_button_panel_size, center=true);
		speed_buttons();
		speed_button_labels();
		for (x=[-1, 1])
			for (y=[-1, 1])
				translate([
					x*(speed_button_panel_size[0]/2 - 15/2),
					y*(speed_button_panel_size[1]/2 - 15/2) + 7.5,
					-1])
				cylinder(r=3/2, h=5, $fn=32);
	}
	% speed_buttons();
	translate([min_th, -min_th, 0])
		speed_button_labels();
}

// ! speed_button_panel();

module menu_buttons() {
	translate([-15, -40, 12])
		button("blue");
	translate([0, -40, 12])
		button("black");
	translate([15, -40, 12])
		button("green");
}

module menu_button_labels() {
	translate([0, -28, 14])
		linear_extrude(label_h/2)
		text(
			"MENU", 
			font="Helvetica:style=Negreta", 
			size=9, 
			halign="center", 
			valign="center");
	translate([24, -40, 12])
		linear_extrude(label_h)
		text(
			"+", 
			font="Helvetica:style=Negreta", 
			size=7, 
			halign="left", 
			valign="center");
	translate([-24, -40, 12])
		linear_extrude(label_h)
		text(
			"-", 
			font="Helvetica:style=Negreta", 
			size=7, 
			halign="right", 
			valign="center");
	translate([0, -46, 12])
		linear_extrude(label_h)
		text(
			"OK", 
			font="Helvetica:style=Negreta", 
			size=4, 
			halign="center", 
			valign="center");
}

lcd_2004_w = 99;
lcd_2004_h = 60;
lcd_2004_d = 10;
lcd_2004_clr = 15;
lcd_2004_th = 2;
lcd_2004_face_h = 40;
lcd_2004_hole_c_c = [93, 55];
lcd_2004_hole_ir = 3/2; //3.5/2;

module lcd_2004_holes() {
	for (x=[-0.5, 0.5])
		for (y=[-0.5, 0.5])
		translate([
			x*lcd_2004_hole_c_c[0],
			y*lcd_2004_hole_c_c[1], -10])
		cylinder(h=60, r=lcd_2004_hole_ir, center=true, $fn=32);
}

module lcd_2004() {
	difference() {
		union() {
			translate([0, 0, lcd_2004_th/2])
				cube([lcd_2004_w, lcd_2004_h, lcd_2004_th], center=true);
			translate([0, 0, lcd_2004_d/2 + lcd_2004_th])
				cube([lcd_2004_w, lcd_2004_face_h, lcd_2004_d], center=true);
			translate([-lcd_2004_w/2+8, lcd_2004_h/2-20, -10])
				cube([47, 20, 10]);
			translate([-lcd_2004_w/2-10, lcd_2004_h/2-16, -10])
				cube([20, 12, 11]);
			// pins: 9mm to 49mm, 5mm tall, 1mm wide, 2.5mm in from long side
			translate([-lcd_2004_w/2+9, lcd_2004_h/2-3, 2])
				cube([40, 1, 6]);
				// TODO: pins are 5 mm, not 6 tall
		}
		lcd_2004_holes();
		// translate([0, 0, 20])
		// 	cube([76, 26, 20], center=true);
	}
}

// ! lcd_2004();

module display_seam() {
	intersection() {
		translate([0, 30, 12])
			rotate([45, 0, 0])
			cube([100, min_th, min_th], center=true);
		// # translate([-51, 30, 0])
		// 	cube([102, 1, 1]);
	}
	intersection() {
		translate([0, -31, 12])
			rotate([45, 0, 0])
			cube([100, min_th, min_th], center=true);
		// translate([-74, -31, 0])
		// 	cube([125, 1, 1]);
	}
	intersection() {
		translate([50, -0.5, 12])
			rotate([0, 45, 0])
			cube([min_th, 61, min_th], center=true);
		// translate([50, -31, 0])
		// 	cube([1, 62, 1]);
	}
	intersection() {
		translate([-50, -0.5, 12])
			rotate([0, -45, 0])
			cube([min_th, 61, min_th], center=true);
		// translate([-74, -31, 0])
		// 	cube([1, 62, 1]);
	}
}

// ! display_seam();

module display_front_face() {
	difference() {
		union() {
			// translate([0, -10, 7])
				// cube([102, 82, 12], center=true);
			translate([0, -10, 13])
				cube([102, 82, 2], center=true);
			// cordgrip_wrap();
			// display_seam();
			
		}
		lcd_2004();
		translate([0, 0, 20])
			cube([76, 26, 20], center=true);
		lcd_2004_holes();
		translate([-62, 0, 0])
			cube([22, 60, 20], center=true);
		translate([0, -25, 5])
			cube([99, 8, 10], center=true);
		translate([0, 25, 5])
			cube([99, 8, 10], center=true);
		menu_buttons();
		translate([0, -41, 6])
			cube([99, 18, 12], center=true);
		// % translate([-63, 0, 0])
		// 	cube([19, 27, 11], center=true);
		// # translate([-62, 0, 0])
		// 	cylinder(r=7/2, h=60, center=true, $fn=64);
		// cordgrip();
		// translate([min_th, -min_th, 0])
			menu_button_labels();
		// display_back_face();
		display_seam();
	}
	translate([min_th, -min_th, 0])
		menu_button_labels();
	% menu_buttons();
}

// ! display_front_face();

module display_back_face() {
	difference() {
		union() {
			// translate([0, -10, -5])
			// 	cube([102, 82, 12], center=true);
			translate([0, -0.5, 0.5])
				cube([102, 63, 23], center=true);
			display_seam();
		}
		// mirror([0, 1, 0])
		// 	display_seam();
		lcd_2004();
		lcd_2004_holes();
		translate([0, 0, -5])
			cube([96, 56, 10], center=true);
		translate([0, -41, 0])
			cube([99, 18, 20], center=true);
		// cordgrip_wrap();
		// cordgrip();
		translate([0, 0, 8])
			cube([lcd_2004_w, lcd_2004_h, 12], center=true);
	}
}

// ! display_back_face();

module display_design(explode=15) {
	// color(alpha=0.2)
	translate([0, 0, explode]) 
		display_front_face();
	// display_front_face_short();
	// color(alpha=0.2) 
	translate([0, 0, -explode])
		display_back_face();
	// display_back_face_short();
	% lcd_2004();
}

display_design(0);

relay2_w = 40;
relay2_h = 50;
relay2_d = 17;
relay2_clr = 3;
relay2_th = 2;
relay2_hole_c_c = [34, 45];
relay2_hole_ir = 3/2;

module relay2_holes() {
	for (x=[-0.5, 0.5])
		for (y=[-0.5, 0.5])
		translate([
			x*relay2_hole_c_c[0],
			y*relay2_hole_c_c[1], 0])
		cylinder(h=50, r=relay2_hole_ir, center=true, $fn=32);
}

module relay2() {
	difference() {
		union() {
			translate([0, 0, relay2_th/2])
				cube([relay2_w, relay2_h, relay2_th], center=true);
			translate([0, 0, relay2_d/2])
				cube([relay2_w-3, relay2_h-3, relay2_d], center=true);
		}
		relay2_holes();
	}
}

// ! relay2();

relay4_w = 74;
relay4_h = 50;
relay4_d = 17;
relay4_clr = 3;
relay4_th = 2;
relay4_hole_c_c = [68, 45];
relay4_hole_ir = 3/2;

module relay4_holes() {
	for (x=[-0.5, 0.5])
		for (y=[-0.5, 0.5])
		translate([
			x*relay4_hole_c_c[0],
			y*relay4_hole_c_c[1], 0])
		cylinder(h=50, r=relay4_hole_ir, center=true, $fn=32);
}

module relay4() {
	difference() {
		union() {
			translate([0, 0, relay4_th/2])
				cube([relay4_w, relay4_h, relay4_th], center=true);
			translate([0, 0, relay4_d/2])
				cube([relay4_w-3, relay4_h-3, relay4_d], center=true);
		}
		relay4_holes();
	}
}

// ! relay4();

toggle_sw_body = [19, 34, 28];
toggle_or = 13/2;
toggle_thread_h = 13;
toggle_top_h = 30;
toggle_top_or = 3;

module toggle_switch() {
	translate([0, 0, -toggle_sw_body[2]/2])
		cube(toggle_sw_body, center=true);
	cylinder(r=toggle_or, h=toggle_thread_h);
	cylinder(r=toggle_top_or, h=toggle_top_h);
}

// ! toggle_switch();

module toggle_cage() {
	difference() {
		translate([-17/2, -6-8, 0])
			cube([17, 41, 2], center=false);
		* translate([-17/2, -6-8-6, 0])
			cube([17, 47, 30], center=false);
		toggle_switch();

	}
}

module m3_spacer() {
	difference() {
		union() {
			cylinder(r1=6/2, r2=5/2, h=3, $fn=32);
			translate([0.5, 0, 0.25])
				cube([6.5, 0.5, 0.5], center=true);
			translate([0, 0.5, 0.25])
				cube([0.5, 6.5, 0.5], center=true);
		}
		translate([0, 0, -1])
			cylinder(r=3.2/2, h=4, $fn=32);
	}

}

// ! m3_spacer();

module m3_spacer_grid() {
	for (x=[-2:2])
		for (y=[-2:2])
		translate([x*6.5, y*6.5, 0])
			m3_spacer();
}

! m3_spacer_grid();

// module stop_sw_face() {
// 	difference() {
// 		union() {
// 			translate([0, 0, relay4_th/2])
// 				cube([relay4_w, relay4_h, relay4_th], center=true);
			
// 		}
// 		translate([-13, 0, 0]) {
// 			# toggle_switch();
// 			% translate([0, 0, relay4_th])
// 				toggle_cage();
// 			translate([0, -15, -1])
// 				linear_extrude(4)
// 				text(
// 					"HEAT", 
// 					font="Helvetica:style=Negreta", 
// 					size=6, halign="center", valign="top");
// 			translate([-10, 10, -1])
// 				linear_extrude(4)
// 				text(
// 					"HIGH", 
// 					font="Helvetica:style=Negreta", 
// 					size=4, halign="right", valign="center");
// 			translate([-10, -10, -1])
// 				linear_extrude(4)
// 				text(
// 					"LOW", 
// 					font="Helvetica:style=Negreta", 
// 					size=4, halign="right", valign="center");
// 			translate([-10, 0, -1])
// 				linear_extrude(4)
// 				text(
// 					"OFF", 
// 					font="Helvetica:style=Negreta", 
// 					size=4, halign="right", valign="center");
// 		}
// 		translate([13, 0, 0]) {
// 			# toggle_switch();
// 			% translate([0, 0, relay4_th])
// 				toggle_cage();
// 			translate([0, -15, -1])
// 				linear_extrude(4)
// 				text(
// 					"PUMP", 
// 					font="Helvetica:style=Negreta", 
// 					size=6, halign="center", valign="top");
// 			translate([10, -10, -1])
// 				linear_extrude(4)
// 				text(
// 					"RUN", 
// 					font="Helvetica:style=Negreta", 
// 					size=4, halign="left", valign="center");
// 			translate([9, 10, -1])
// 				linear_extrude(4)
// 					text(
// 					"STOP", 
// 					font="Helvetica:style=Negreta", 
// 					size=4, halign="left", valign="center");
// 		}
// 		relay4_holes();
// 	}
// 	% translate([0, 0, -tall_standoff_h - tall_standoff_threads+1]) {
// 		relay4();
// 		electronics_mount(relay4_hole_c_c);
// 	}
// }


// module valve_sw_face() {
// 	difference() {
// 		union() {
// 			translate([0, 0, relay4_th/2])
// 				cube([relay4_w, relay4_h, relay4_th], center=true);
			
// 		}
// 		translate([-11, 0, 0]) {
// 			# toggle_switch();
// 			% translate([0, 0, relay4_th])
// 				toggle_cage();
// 			translate([0, -15, -1])
// 				linear_extrude(4)
// 				text(
// 					"IN", 
// 					font="Helvetica:style=Negreta", 
// 					size=6, halign="center", valign="top");
// 			translate([-10, 10, -1])
// 				linear_extrude(4)
// 				text(
// 					"SPA", 
// 					font="Helvetica:style=Negreta", 
// 					size=4, halign="right", valign="center");
// 			translate([-10, -10, -1])
// 				linear_extrude(4)
// 				text(
// 					"POOL", 
// 					font="Helvetica:style=Negreta", 
// 					size=4, halign="right", valign="center");
// 			translate([-10, 0, -1])
// 				linear_extrude(4)
// 				text(
// 					"AUTO", 
// 					font="Helvetica:style=Negreta", 
// 					size=4, halign="right", valign="center");
// 		}
// 		translate([11, 0, 0]) {
// 			# toggle_switch();
// 			% translate([0, 0, relay4_th])
// 				toggle_cage();
// 			translate([0, -15, -1])
// 				linear_extrude(4)
// 				text(
// 					"OUT", 
// 					font="Helvetica:style=Negreta", 
// 					size=6, halign="center", valign="top");
// 			translate([10, -10, -1])
// 				linear_extrude(4)
// 				text(
// 					"POOL", 
// 					font="Helvetica:style=Negreta", 
// 					size=4, halign="left", valign="center");
// 			translate([9, 10, -1])
// 				linear_extrude(4)
// 					text(
// 					"SPA", 
// 					font="Helvetica:style=Negreta", 
// 					size=4, halign="left", valign="center");
// 			translate([9, 0, -1])
// 				linear_extrude(4)
// 				text(
// 					"AUTO", 
// 					font="Helvetica:style=Negreta", 
// 					size=4, halign="left", valign="center");
// 		}
// 		relay4_holes();
// 	}
// 	% translate([0, 0, -tall_standoff_h - tall_standoff_threads+1]) {
// 		relay4();
// 		electronics_mount(relay4_hole_c_c);
// 	}
// }

// ! stop_sw_face();



// module display_front_face_short() {
// 	intersection() {
// 		display_front_face();
// 		translate([0, 0, 10])
// 			cube([200, 200, 1], center=true);
// 	}
// }

// // ! display_front_face_short();

// module display_back_face_short() {
// 	intersection() {
// 		display_back_face();
// 		translate([0, 0, 0])
// 			cube([200, 200, 1], center=true);
// 	}
// }

// // ! display_back_face_short();

// bbox_size = [250, 150, 100];

// // bbox_holes = [
// // 	[-64, 10],
// // 	[64, 10],
// // 	[-197/2, 112],
// // 	[197/2, 112],
// // 	[-30, 112],
// // 	[30, 112],
// // ];

// bbox_holes = [
// 	[-64, 1/2 * IN_MM],
// 	[64, 1/2 * IN_MM],
// 	[-7.75*IN_MM/2, 4.5 * IN_MM],
// 	[7.75*IN_MM/2, 4.5 * IN_MM],
// 	[-(2+3/8)*IN_MM/2, 4.5 * IN_MM],
// 	[(2+3/8)*IN_MM/2, 4.5 * IN_MM],
// ];
// bbox_hole_r = 5/2;

// module breaker_box_holes() {
// 	for (p=bbox_holes) {
// 		translate([p[0], bbox_size[1]/2-p[1], 0])
// 			cylinder(r=bbox_hole_r, h=bbox_size[2]*2, center=true, $fn=32);
// 	}
// }

// // breaker_box_holes();

// module breaker_box() {
// 	% difference() {
// 		translate([0, 0, -bbox_size[2]/2-1]) 
// 			cube([bbox_size[0], bbox_size[1], 2], center=true);
// 		breaker_box_holes();
// 	}
// }

// // % breaker_box();

// bbox_mount_th = 2;

// module breaker_box_mount() {
// 	difference() {
// 		union() {
// 			for (p=bbox_holes) {
// 				translate([p[0], 150/2-p[1], 0])
// 					cylinder(r=bbox_hole_r*2, h=bbox_mount_th, center=false, $fn=32);
// 			}
// 			translate([-100, bbox_size[1]/2 - bbox_holes[2][1] - 5, 0])
// 				cube([200, 10, bbox_mount_th], center=false);
// 			translate([bbox_holes[0][0], bbox_size[1]/2 - bbox_holes[0][1] - 5, 0])
// 				cube([abs(bbox_holes[0][0]*2), 10, bbox_mount_th], center=false);
// 			translate([bbox_holes[0][0], bbox_size[1]/2 - bbox_holes[0][1], 0])
// 				rotate([0, 0, -108.5])
// 				translate([54, 0, bbox_mount_th/2])
// 				cube([108, 10, bbox_mount_th], center=true);
// 			translate([bbox_holes[0][0], bbox_size[1]/2 - bbox_holes[0][1], 0])
// 				rotate([0, 0, -71.5])
// 				translate([54, 0, bbox_mount_th/2])
// 				cube([108, 10, bbox_mount_th], center=true);
// 			translate([bbox_holes[1][0], bbox_size[1]/2 - bbox_holes[1][1], 0])
// 				rotate([0, 0, -108.5])
// 				translate([54, 0, bbox_mount_th/2])
// 				cube([108, 10, bbox_mount_th], center=true);
// 			translate([bbox_holes[1][0], bbox_size[1]/2 - bbox_holes[1][1], 0])
// 				rotate([0, 0, -71.5])
// 				translate([54, 0, bbox_mount_th/2])
// 				cube([108, 10, bbox_mount_th], center=true);
// 			translate([-85, 11.5, 0])
// 				cube([170, 10, bbox_mount_th], center=false);
// 			translate([-62, -50, bbox_mount_th/2])
// 				cube([10, 35, bbox_mount_th], center=true);
// 			translate([62, -50, bbox_mount_th/2])
// 				cube([10, 35, bbox_mount_th], center=true);
			
// 		}
// 		breaker_box_holes();
// 	}
// }

// // breaker_box_mount();

// module electronics() {
// 	translate(valve_relay_offset)
// 		% relay4();
// 	translate(pump_relay_offset)
// 		% relay4();
// 	translate(clean_relay_offset)
// 		rotate(clean_relay_rotation)
// 		% relay2();
// 	translate(acdc_offset)
// 		% acdc();
// 	translate(tblock_offset)
// 		% tblock();
// 	translate(arduino_offset)
// 		% arduino();
// }

// // ! electronics();

// electronics_holes = [
// 	relay2_hole_c_c,
// 	relay4_hole_c_c,
// ];
// electronics_hole_r = 3/2;

// module electronics_holes(p, ir=electronics_hole_r) {
// 	translate([p[0]/2, p[1]/2, 5/2]) {
// 		cylinder(r=ir, h=6, center=true, $fn=32);
// 	}
// 	translate([p[0]/2, -p[1]/2, 5/2])
// 		cylinder(r=ir, h=6, center=true, $fn=32);
// 	translate([-p[0]/2, p[1]/2, 5/2])
// 		cylinder(r=ir, h=6, center=true, $fn=32);
// 	translate([-p[0]/2, -p[1]/2, 5/2])
// 		cylinder(r=ir, h=6, center=true, $fn=32);
// }

// module electronics_mount(p, ir=electronics_hole_r, or=5) {
// 	difference() {
// 		union() {
// 			translate([p[0]/2, p[1]/2, 5/2]) {
// 				% translate([0, 0, 2.5])
// 					tall_standoff();
// 				cylinder(r1=or, r2=ir+1, h=5, center=true, $fn=32);
// 			}
// 			translate([-p[0]/2, p[1]/2, 5/2]) {
// 				% translate([0, 0, 2.5])
// 					tall_standoff();
// 				cylinder(r1=or, r2=ir+1, h=5, center=true, $fn=32);
// 			}
// 			translate([p[0]/2, -p[1]/2, 5/2]) {
// 				% translate([0, 0, 2.5])
// 					tall_standoff();
// 				cylinder(r1=or, r2=ir+1, h=5, center=true, $fn=32);
// 			}
// 			translate([-p[0]/2, -p[1]/2, 5/2]) {
// 				% translate([0, 0, 2.5])
// 					tall_standoff();
// 				cylinder(r1=or, r2=ir+1, h=5, center=true, $fn=32);
// 			}
// 		}
// 		electronics_holes(p);
// 	}
// }

// acdc_offset = [0, -10, 0];
// tblock_offset = [0, -60, 0];
// arduino_offset = [-25, 44, 0];
// valve_relay_offset = [52.5, -10, 0];
// pump_relay_offset = [-52.5, -10, 0];
// clean_relay_offset = [36, 40, 0];
// clean_relay_rotation = [0, 0, 90];

// module box_plate() {
// 	% translate([0, 0, -bbox_size[2]/2+5])
// 		electronics();
// 	difference() {
// 		union() {
// 			translate([0, 0, -bbox_size[2]/2]) {
// 				breaker_box_mount();
// 				translate(pump_relay_offset)
// 					electronics_mount(relay4_hole_c_c);	
// 				translate(valve_relay_offset)
// 					electronics_mount(relay4_hole_c_c);	
// 				translate(clean_relay_offset)
// 					rotate(clean_relay_rotation)
// 					electronics_mount(relay2_hole_c_c);	
// 				translate(acdc_offset)
// 					electronics_mount(acdc_hole_c_c);
// 				translate(tblock_offset)
// 					electronics_mount(tblock_hole_c_c);
// 				translate(arduino_offset)
// 					electronics_mount(arduino_hole_c_c);	
// 			}
// 		}
// 		translate([0, 0, -bbox_size[2]/2]) {
// 			translate(pump_relay_offset)
// 				electronics_holes(relay4_hole_c_c);
// 			translate(valve_relay_offset)
// 				electronics_holes(relay4_hole_c_c);	
// 			translate(clean_relay_offset)
// 				rotate(clean_relay_rotation)
// 				electronics_holes(relay2_hole_c_c);	
// 			translate(acdc_offset)
// 				electronics_holes(acdc_hole_c_c);
// 			translate(tblock_offset)
// 				electronics_holes(tblock_hole_c_c);
// 			translate(arduino_offset)
// 				electronics_holes(arduino_hole_c_c);
// 		}
// 	}
// 	// translate([-intermatic_w/2, -intermatic_l/2, 0])
// 	// 	intermatic_face();
// 	// translate([0, 0, intermatic_h/2])
// 	// 	front_face();
// 	// translate([0, 0, -intermatic_d])
// 	// 	back_face();
// 	// % translate([0, 0, -intermatic_d])
// 	// 	cube([intermatic_w, intermatic_l, intermatic_d]);
// 	// % translate([0, 0, 0])
// 	// 	cube([intermatic_w, intermatic_l, intermatic_h]);
// }

// module design() {
// 	box_plate();
// 	// % breaker_box();
// 	translate(pump_relay_offset + [0, 0, 10])
// 		stop_sw_face();
// 	translate(valve_relay_offset + [0, 0, 10])
// 		valve_sw_face();
// }

// design();

// module box_plate_short() {
// 	intersection() {
// 		box_plate();
// 		translate([0, 0, -bbox_size[2]/2 + 1])
// 			cube([250, 250, 1], center=true);
// 	}
// }

// // box_plate_short();
