(* Input/Output *)

print_endline "Please enter your name: ";
let name = read_line () in
print_string "Hello, ";
print_string name;
print_endline "!";
print_endline "What's your favorite number? ";
let num = read_int () in
print_string "I like ";
print_int num;
print_endline " too!";
print_endline "Do you know pi? ";
let pi = read_float () in
print_string "You said ";
print_float pi;
print_string ", I say ";
print_float 3.141592653589793
