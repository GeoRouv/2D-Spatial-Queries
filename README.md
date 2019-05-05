# 2D-Spatial-Queries
Five simple utilities that implement spatial queries in two-dimensional space

## Description
The goal of this work is to familiarize with the creation of process hierarchies using the fork() system call that in combination with the exec*() call can help differentiate and share work. In addition, arious system calls such as wait, mk fi fo, fread, fwrite, I / Os, simple synchronization, and communication between processes are being used.

## Compile
    $ make

## Run

    $ ./shapes -i InputBinaryFile -w WorkersCount -d TempDir

Command line options (in any order):<br>
* w : number of workers (must be greater than 2, see end of readme)<br>
* i : filename input<br>
* (optional) d : output file name<br>

Example:

    $ ./shapes -w 3 -i points_4000_gridsize_100.bin -d TempDir

## Implementation Notes

### Utilities

All utilities can be executed with the following arguments:<br>

    $ ./utilityX -i InputBinaryFile -o OutputFile -a UtilityArgs [-f Offset] [-n PointsToReadCount]

where:<br>
* utilityX is the one of the five utilities to implement; 
* InputBinaryFile is the binary file with the serialized data in fl oat, float, ... (no space or comma)
* OutputFile the output file that will contain in ASCII format the points that satisfy the query of each utility, with each point per line and its coordinates separated by TAB. 
* UtilityArgs the arguments of each utility (detailed below) separated by a blank space between them 
* The Offset is the number of bytes to be ignored at the beginning of the file. This parameter is optional and is therefore surrounded by parentheses [], 
* PointsToReadCount is the number of points (pairs of fl oat numbers) that the utility reads from the input file. This parameter is optional and is therefore surrounded by parentheses []

The -i / -o / -a / -f / -n flags can be used in any order in the program execution bar. Each of the utilities is further qualified by the -a parameter depending on its function, as follows: 

1. circle -a x y r<br>
Since the program is the circle, it is checked whether the entry point belongs to the circle ([x , y], r).
2. semicircle -a x y r N / S / W / E<br>
Since the program of use is the semicircle, it is checked whether the entry point belongs to the circle ([x, y], r), namely North, South, West, East semicircle. 
3. ring -a x y r1 r2<br>
If the utility is the ring, then each entry point is checked if it belongs to the ring ([x, y], r1, r2). 
4. square -a x y r<br>
If the utility is square then each entry point is checked if it belongs to the square ([x, y], r) which has a vertical and a horizontal diagonal. 
5. ellipse -a h k α β<br>
Since the program of use is the ellipse then each entry point is checked if it belongs to the shortage defined by the center C (h, k), it has a large axis x and y is small, and whose equation is the following: (x-h) 2 a2 + (y-k) 2 b2 = 1.
The order that the parameters provided in the above utilities are strict (ie, a sequence change implies a different two-dimensional space).

Below, you see the queries visualized in full correspondence with the list above:

![Screenshot](Screenshot_2.png)
 
### shapes.c

	-Κάθε φορά που δέχεται μια σειρά από εντολές το πρόγραμμα, τις φυλάει σε ένα 2D array.
	-Κάθε εντολή(string) αυτού του array την κάνω tokenize(δηλαδή τη χωρίζω σε tokens) και τα αποθηκεύω σε έναν 3D array όπου σε κάθε κελί βρίσκεται ένα από αυτά.
	-Όσες είναι οι εντολές που λαμβάνω, τόσους handlers φτιάχνω.
	-Εάν ο αριθμός των δοθέντων συντεταγμένων μπορεί να χωριστεί εξίσου στον αριθμό των workers όλα καλά, ειδάλλως κάθε worker παίρνει ένα extra ζευγάρι συντεταγμένων εκτός από τον τελευταίο που παίρνει 		λιγότερα.
	-Πριν να εκτελέσει ο worker το utility που του αντιστοιχεί φτιάχνει το fifo file του και αν είναι ο parent το ανοίγει για διάβασμα και περιμένει την εκτέλεση του utility.Αν είναι child τότε εκτελεί 		το utility.
	-Όταν τελειώσουν όλοι οι workers τότε ο handler μαζεύει τις πληροφορίες από όλα τα fifos και τις γράφει σε ένα .οut.(Θα μπορούσε να γίνει πιο αποδοτικά ξεκινώντας τη συλλογή πληροφοριών από το fifo 		του worker που τελείωσε πρώτος με τη βοήθεια του pid του).
	-To gnuplot script δημιουγεί κανονικά την .png εικόνα στο directory που βρίσκεται και τα fifos δημιουργούνται στο TempDir που έχει δοθεί ώς αρχική παράμετρος.
	-Με εκτέλεση της εντολής exit στο CLI Mode του shapes τερματίζει το πρόγραμμα.
	-Με εκτέλεση της εντολής make clean στο κανονικό command line καθαρίζονται τα εκτελέσιμα όπως και ο TempDir που έχει δημιουργηθεί από προηγούμενη εκτέλεση.	


-Σημείωση: Το πρόγραμμα για αριθμό w <= 2 δε λειτουργεί ορθά, κολλώντας μετά την εκτέλεση του τελευταίου worker.Για οποιοδήποτε άλλο όμως αριθμό το πρόγραμμα λειτουργεί σωστά και για οποιαδήποτε δεδομένα.

-Output PNG's: o φάκελος αυτός έχει αποτελέσματα για τις ακόλουθες εκτελέσεις:
	
	6_image.png: ring 2 3 80 60 red;  							(gridsize 4000)
	7_image.png: circle 2 3 50 blue;  							(gridsize 4000)
	0_image.png: ellipse 2 3 1000 2000 green;					(gridsize 8000)
	2_image.png: semicircle 500 500 1000 N purple; 				(gridsize 8000)
