function compute_stats()
%make sure 1 is the first number in the thread list
doors = [16];
agents = [8];
mrows = [25];

for i=1:50
  filename = ['/usr0/home/aebtekar/Research/MultiAgentSearch/stats/complete_' num2str(doors(1)) '_' num2str(agents(1)) '_' num2str(mrows(1)) '_' num2str(i) '.csv'];
  data = load(filename);
  cost(i) = data(2);
  expands(i) = data(3);
  time(i) = data(4);
  jtime(i) = data(5);
  
  filename = ['/usr0/home/aebtekar/Research/MultiAgentSearch/stats/greedy_' num2str(doors(1)) '_' num2str(agents(1)) '_' num2str(mrows(1)) '_' num2str(i) '.csv'];
  data = load(filename);
  gcost(i) = data(2);
  gexpands(i) = data(3);
  gtime(i) = data(4);
end

format long g;
doors
agents
mrows

v = sort(time);
v(14)
v(37)
0.5 * (v(25)+v(26))
2*sum(v < 5)

v = sort(gtime);
v(14)
v(37)
0.5 * (v(25)+v(26))
2*sum(v < 5)

%fprintf('%d %d %d\n',mean(cost), mean(expands), mean(time));
