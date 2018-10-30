clear 
close all
clc

P=[514.441 174.572 -3320.65 193451;
-44.4847 3340.13 134.409 -64709;
0.993466 0.103531 0.0480279 400.192;
];

K=[3.34442609e+03 -7.91484993e-05  3.69669403e+02
0.00000000e+00  3.32890477e+03  3.08068338e+02
0.00000000e+00  0.00000000e+00  1.00000002e+00];
E=[0.0440096 0.0407543 -0.9982;
-0.105302 0.993791 0.0359317;
0.993466 0.103531 0.0480279;
13.6084 -56.4737 400.192]';

E2=roty(-pi/2)*E;

P2=K*E2



