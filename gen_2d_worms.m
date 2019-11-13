
function gen_2d_worms(ds_id,max_clusters)
% max_clusters: Generate no more than max_clusters clusters. 
% Might be significantly less because of collisions

numsteps=200;
labels=[];
i_clu=1;
i=1; X = []; c_trail = []; num_noisep=8; nump=16;
for i_shape=1:max_clusters
    steepness=1+rand()*6;
    numsteps=randi([100 300],1,1);
    var_range=[10 80];
    c = rand(1,2)*2000-1000;
    [X,c_trail,labels,i_clu] = gen_one_worm(X,c,var_range,numsteps,steepness,nump,i_clu,num_noisep,c_trail,labels);
end
minX=min(X(:));
X=X-minX;

dsfn=sprintf('worms_%d.txt',ds_id);
pngfn=sprintf('worms_%d.png',ds_id);
dlmwrite(dsfn,X,'Delimiter',' ','precision',6);
fprintf('Dataset file generated: %s\n',dsfn);

% Labels, first column = class, second column = dense or sparse component
label_fn=sprintf('worms_%d-labels.txt',ds_id);
dlmwrite(label_fn,round(labels),'Delimiter',' ');
fprintf('Ground truth file generated: %s\n',label_fn);

gpscriptfn=sprintf('gnuplot_script_%d.txt',ds_id);
gpscriptfn=sprintf('gnuplot_script_%d.txt',ds_id);
fh = fopen(gpscriptfn,'w');

xsize = round(2048*(max(X(:,1))-min(X(:,1)))/(max(X(:,2))-min(X(:,2))));
fprintf(fh,'set terminal pngcairo size %d,2048 enhanced font "Verdana,10"\n',xsize);
fprintf(fh,'set size ratio -1\n');
fprintf(fh,'set output "%s"\n',pngfn);
fprintf(fh,'set style fill  transparent solid 0.40 noborder\n');
fprintf(fh,'set style circle radius 5\n');
fprintf(fh,'plot "%s" u 1:2 with circles lc rgb "black"\n',dsfn);

fclose(fh);
fprintf('To visualize, run: gnuplot %s\n',gpscriptfn);
end


function [X,c_trail,labels,i_clu] = gen_one_worm(X,c,var_range,numsteps,steepness,nump,i_clu,num_noisep,c_trail,labels)

stepl=5;
dims=2;
new_cs=[];
num_rdirs=3;
Xnew=[];
labels_new=[];
for j=1:num_rdirs
    x = rand(1,dims)-ones(1,dims)*0.5;
    x = x/norm(x);
    rdir(j,:) = x;
end
i=1;
v = rdir(1,:);
for i_step=1:numsteps
    p = i_step/numsteps;
    v2 = v*[0 1;-1 0];
    v = v+v2*(steepness/numsteps);
    v = v/norm(v);
    varr = var_range(1)*(1-p) + var_range(2)*(p);
    c = c+v*stepl;
    
    if ~isempty(c_trail)
        [idx,d] = knnsearch(c_trail,c);
        % Collision detected
        if d < 50
            break;
        end
    end
    
    new_cs = [new_cs; c];
    
    for j=1:nump
        b=normrnd(c(:),varr);
        Xnew(i,:) = b;
        labels_new(i,:) = [i_clu,1];
        i = i + 1;
    end
    
    for j=1:num_noisep
        b=normrnd(c(:),varr*8);
        Xnew(i,:) = b;
        labels_new(i,:) = [i_clu,2];
        i = i + 1;
    end
    
    
end

if i_step > numsteps/4
    X = [X; Xnew];
    c_trail = [c_trail; new_cs];
    labels=[labels;labels_new];
    i_clu=i_clu+1;
    
end
end



