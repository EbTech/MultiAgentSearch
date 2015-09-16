function compute_stats()
%make sure 1 is the first number in the thread list
doors = [7];
agents = [7];
mrows = [13];

for i=1:10
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
agents

st = sort(time)
0.5 * (st(5)+st(6))
st = sort(expands);
0.5 * (st(5)+st(6))
%jtime
st = sort(gtime)
0.5 * (st(5)+st(6))
st = sort(gexpands);
0.5 * (st(5)+st(6))
cost
gcost

%fprintf('%d %d %d\n',mean(cost), mean(expands), mean(time));
