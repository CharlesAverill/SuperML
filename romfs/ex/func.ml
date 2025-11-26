(* Functions *)

let f x y = sub (mul (add x y) 2) 4 in
print_int (f 20 3); print_endline "";

let g x y z = concat x z in
print_endline (g "Hello " "Cruel " "World!")
