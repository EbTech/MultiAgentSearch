function compute_stats()
%make sure 1 is the first number in the thread list
doors = [7];
agents = [7];
mrows = [17];


for i=1:10
  filename = ['/usr0/home/aebtekar/Research/MultiAgentSearch/stats/complete_' num2str(doors(1)) '_' num2str(agents(1)) '_' num2str(mrows(1)) '_' num2str(i) '.csv'];
  data = load(filename);
  cost(i) = data(2);
  expands(i) = data(3);
  time(i) = data(4);
end

cost
expands
time

%fprintf('%d %d %d\n',mean(cost), mean(expands), mean(time));
