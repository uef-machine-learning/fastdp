

function gen_high_dim_worms(ds_id,max_clusters,dims)

numsteps=200;
labels=[];
i_clu=1;
i=1; X = []; c_trail = []; num_noisep=0; nump=16;

for i_shape=1:max_clusters
    steepness=1;
    numsteps=300;
    nump=10;
    stepl=2;
    num_noisep=4;
    noisemultip=4;
    var_range=[10 50]*20.0;
    c = rand(1,dims)*2000-1000;
    [X,c_trail,labels,i_clu] = gen_one_hd_worm(X,c,var_range,numsteps,steepness,nump,i_clu,num_noisep,c_trail,labels,dims,stepl,noisemultip);

end

numsamples=5000;
rp=randperm( size(X,1));
Xs=X(rp(1:numsamples),:);
d=pdist(Xs);
close all;
hist(d,200);

minX=min(X(:));
X=X-minX;
% size(X)

dsfn=sprintf('worms_%d.txt',ds_id);
pngfn=sprintf('worms_%d.png',ds_id);
dlmwrite(dsfn,X,'Delimiter',' ','precision',6);
fprintf('Dataset file generated: %s\n',dsfn);
% Labels, first column = class, second column = dense or sparse component
label_fn=sprintf('worms_%d-labels.txt',ds_id);
dlmwrite(label_fn,round(labels),'Delimiter',' ');
fprintf('Ground truth file generated: %s\n',label_fn);

end


function [X,c_trail,labels,i_clu] = gen_one_hd_worm(X,c,var_range,numsteps,steepness,nump,i_clu,num_noisep,c_trail,labels,dims,stepl,noisemultip)


new_cs=[];
num_rdirs=3;
Xnew=[];
labels_new=[];
rdir=[];
for j=1:num_rdirs
    x = rand(1,dims)-ones(1,dims)*0.5;
    x = x/norm(x);
    rdir(j,:) = x;
end
i=1;
v = rdir(1,:);
orthos=null(v(:).')';

% Random vector orthogonal to v
v3=sum(orthos'*diag(rand(1,dims-1)),2);v3=v3/norm(v3);

for i_step=1:numsteps
    p = i_step/numsteps;
    
    orthos=null(v(:).')';
    % Random vector orthogonal to v
    v3=sum(orthos'*diag(rand(1,dims-1)),2);v3=v3/norm(v3);
    
    v2=v3;
    
    v = v+v2'*(steepness/numsteps);
    v = v/norm(v);
    varr = var_range(1)*(1-p) + var_range(2)*(p);
    c = c+v*stepl;
    
    if ~isempty(c_trail)
        [idx,d] = knnsearch(c_trail,c);
        if d < 50
            fprintf('[%d] Got too close to other cluster\n',i_clu);
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
        b=normrnd(c(:),varr*noisemultip);
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



