function pinequalitysurf(p)
% PINEQUALITYSURF  help with proving p-monotone inequality in Appendix by plotting

f = @(t,s,p) (1 - (t.^(p-1) + t) .* s + t.^p) ./ (1 - 2*s.*t + t.^2).^(p/2);

tt = 0:.01:1;
ss = -1:.01:.99;
[ttt,sss]=meshgrid(tt,ss);

figure(1)
zzz = f(ttt,sss,p);
surf(sss,ttt,zzz), colorbar, shading('interp')
xlabel s, ylabel t
title('f(t,s)')
fprintf('f(t,s) bounded below by %.6f, versus 2^(2-p) = %.6f\n',min(min(zzz)),2^(2-p))

figure(2)
t = 0:.01:.99;
plot(t,(1-t.^(p-1))./(1-t).^(p-1))
xlabel t
axis([0 1 0 4])

return   % comment out to see more figures

figure(3)
g = @(t,s,p) s .* (2-p) .* t .* (t.^(p-2) + 1) + ((p-1) * (t.^p+1) - (t.^(p-2) + t.^2));
surf(sss,ttt,g(ttt,sss,p)), colorbar, shading('interp')
title('g(t,s) = df/ds / (positive factor)')

figure(4)
plot(tt,(p-1)*(tt.^p+1),'r',tt,(p-2)*(tt.^(p-1)+tt)+tt.^(p-2)+tt.^2,'b')
legend('should be above','should be below','location','southeast')

