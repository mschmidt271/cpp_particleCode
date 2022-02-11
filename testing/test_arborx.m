clear variables

N = 1e4;
dist = 2.268;

X = linspace(1, 10, N)';
% dist = X - X';
% neighb = abs(dist) < 1;

[idx, r] = rangesearch(X, X, dist);

fid = fopen('cpp.txt', 'r');
tline = fgetl(fid);
cppidx = cell(0,1);
while ischar(tline)
    cppidx{end+1,1} = str2double(split(tline, ', '))';
    tline = fgetl(fid);
end
fclose(fid);

for i = 1 : 30
    cppidx{i} = sort(cppidx{i});
    idx{i} = sort(idx{i});
end

C = cellfun(@minus,cppidx,idx,'Un',0);
d = cellfun(@sum, C)
sum(d)