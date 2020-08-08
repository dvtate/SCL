
let while = (:
	let cond = i[0], act = i[1], break = o
	if (cond(break), (:
		act(break)
		while(cond, act)
	))
)
	
let foreach = (:
	let list = i[0], action = i[1]
	let index = 0, end = size(list)
	while((: index < end), (:
		action(list[index], index, i)
		index = index + 1	
	))
)

/*

let map = (:
	let list = i[0], fn = i[1]
	let list = copy(i[0])
	let ret = foreach(i[0], (: ret[i[1]] = fn(i)))
	if(ret == empty, list, ret)
)

let split = (: 
	let s = i[0], delim = i[1]
	let ret = [""], ind = 0

	let is_delim = (:
		let c = i, ret = o
		foreach(delim, (: 
			if (i[0] == c, (: 
				ret(1) 
			))
		))
	)

	foreach(s, (:
		if (is_delim(i[0]), (:
			ret = ret + ""
		), (:

		))
	))
);
*/

let world = [
	"                                   ",
	"    ###       #                    ",
	"   ###       # #           #       ",
	"              #            #       ",
	"    ##                     #       ",
	"    #                ##            ",
	"     ###             ##            ",
	"       #               ##          ",
	"                       ##          "
]

// Standins for proper shortcircuit operators
let or =  (: if (i[0], i[0], i[1]) );
let and = (: if (i[0], i[1], 0   ) );

let neighbors = (:
	let x = i[0], y = i[1];
	
	let s = and(world[x + 1], (: world[x + 1][y] == '#' ));
	let n = and(x > 0, (: world[x - 1][y] == '#' ));
	let w = world[x][y - 1] == '#';
	let e = world[x][y + 1] == '#';
	let se = and(world[x + 1], (: world[x + 1][y + 1] ));
	let sw = and(world[x + 1], (: world[x + 1][y - 1] ));
	let ne = and(x > 0, (: world[x - 1][y + 1] ));
	let nw = and(x > 0, (: world[x - 1][y - 1] ));
	
	//print(up, down, left, right);

	let ret = 0;
	foreach([n, s, e, w, ne, nw, se, sw], (: 
		ret = ret + Num(i[0])
	));
	o(ret);
)

let print_world = (: 
	foreach(world, (:
		print(i[0], empty());
	))
);

let update_world = (:
	let ret = copy(world);
	
	foreach (world, (:
		let row = i[1];
		print(world[row]);
		ret[row] = "";
		//print(world[row]);
		foreach(world[row], (:
			let col = i[1];

			let ns = neighbors(row, col);
			
			//print('cell', row, col, "neighbors", ns);
			//print(world[row], world[row][col], ns);
	//		print('i', i);
			ret[row] = ret[row] + if (
				or(
						and(i[0] == ' ', ns == 3),
						and(i[0] == '#', or(ns == 2, ns == 3))),
					'#', ' '
				);			
		))
	))
	world = ret
);

print_world()
update_world()
print('----------------------------')

print_world()
print('----------------------------')

