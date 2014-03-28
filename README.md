# qubs - A small and simple RPN QBF solver

qubs takes a quantified Boolean formula in reverse polish notation
(postfix notation) from STDIN and searches for a solution.

## Command line arguments

```
./qubs [ -s | -t | -c ] [-v[v][v][v]]

	-s	satisfiability (default)
	-t	tautology
	-c	contradiction
	-i	interactive
	-v	verbose (can be specified multiple times)
		1 =	query written to terminal
		2 =	plus progress output
		3 =	plus optimized BDD output
		4 =	plus unoptimized BDD output
```

The exit code also reflects the result (according to the selected evaluation mode):

* 0 = false
* 1 = true

## Source file syntax

Every single argument is placed on one line. The complete input must
be parsed successfully and must be completely specified.

### Keywords

### AND

Logical AND has 2 arguments. It evaluates to true if both arguments are true.

### OR

Logical OR has 2 arguments. It evaluates to true if one of both arguments is true.

### NOT

Logical NOT has 1 argument. It evaluates to true if the argument is not true.

### EXISTS

Existential quantifier has 2 arguments (first one is a variable). Evaluates to true
if there is a variable value of the first argument that can satisfy the second argument.

### FORALL

Universal quantifier has 2 arguments (first one is variable). Evaluates to true
if all variable values of the first argument that satisfy the second argument.

### IMPLIES (or IF)

The logical implication has 2 arguments. It evaluates to false, if there is a variable
constellation that satisfies the first argument but cannot satisfy the second one.

### EQUIVALENT (or IFF)

The logical equivalence (equality) has 2 arguments. It evaluates to true, if first argument
implies the second and vice versa with same variable constellation.

## Examples

You can find some examples in the directory [examples](https://github.com/nakal/qubs/tree/master/examples).

There are also more sophisticated examples in the subdirectories:

* [examples/3sat](https://github.com/nakal/qubs/tree/master/examples/3sat) - 3-SAT problem generator
* [examples/nqueens](https://github.com/nakal/qubs/tree/master/examples/nqueens) - n-queens problem generator

### Simple example

```
EXISTS
x
AND
x
NOT
x
```

Asks if for any values of x we can find both x true and x false. The answer is of course no.

```
# qubs -v
satisfiable: no
```

```
# qubs -vt
tautology: no
```

```
# qubs -vc
contradiction: yes
```


