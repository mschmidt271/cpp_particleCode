clear variables

load('test_pts.mat');

[idx, r] = rangesearch(X, X, dist);

fid = fopen('cpp_results.txt', 'r');
tline = fgetl(fid);
cppidx = cell(N, 1);
pN = 1;
while ischar(tline)
    cppidx{pN,1} = str2double(split(tline, ', '))';
    tline = fgetl(fid);
    pN = pN + 1;
end
fclose(fid);

for i = 1 : N
    cppidx{i} = sort(cppidx{i});
    idx{i} = sort(idx{i});
end

C = cellfun(@minus,cppidx,idx,'Un',0);
err = sum(cellfun(@sum, C))
