clear variables

% dim = 1;
dim = 3;
N = 1e3;
L = 5;
% dist = 2.268;
dist = 1.132;

% 1d, equi-spaced
% pts = linspace(1, 10, L)';
% 2d, random scatter
pts = L * rand(N, dim);
X = pts;

save('test_pts.mat', 'N', 'dim', 'L', 'dist', 'X');