现有serverA（通用电信网下的服务器）,serverB,serverC,serverD,serverE,clientA(通用电信网下的客户端),clientB(通用电信网下的客户端),clientX,clientY
1.环境的初步建立
  用clientA向serverA的9930，9931，9932这三个端口发送一次请求
  serverA可以收到3条来自clientA的请求，获取这3条请求的ip地址和端口号，分别是【ip1:port1】,【ip2:port2】,【ip3:port3】
  [判断ip1，ip2，ip3是否相等]
  在通常情况下，clientA所在的网络环境没有改变，这3条请求的ip应该是相同的，如果不相同，则作废这三条请求，用serverA的9933端口发送命令给clientA，提示clientA端重新发送。
  [判断port1，port2，port3是否相等]
  1.如果这3条请求的3个端口号完全相同，可以认为在clientA侧，路由出时，端口号固定不变
  2.如果这3条请求的3个端口号之间存在一定规律变化(22345，22346，22347)，可以认为在clientA侧，路由出时，端口号以一定规律变化(+1)
  3.如果这3条请求的3个端口号之间杂乱无章，可以认为在clientA侧，路由出时，端口号随机分配
  在serverA端记录下clientA的3条请求，记录形式可以是 
  ---------------------------------------------------------------
  clientA | out  | type| ip1:port1  |  ip2:port2  |  ip3:port3  |
  ---------------------------------------------------------------
  记录完毕后，发送给其他server进行信息共享
  如果ip1，ip2，ip3完全相等，用serverA的9930，9934，9935各发送一条返回消息给clientA，消息内容为某一个特定信号(ASDFGH);同时serverA发送[新Client路由入性质判断消息]给serverB，提示serverB发送消息给clientA，serverB在接收到serverA的提示后，以9930端口向clientA发送一条请求，消息内容为某一个特定信号(ASDFGH)
  在clientA端,如果接收到请求内容为ASDFGH的消息，则发送接收到消息的【ip:port ASDFGH】给serverA【一共会发送4条消息给clientA,如果中间clientA的网络环境发生变化，比如接收到serverA9930端口发来的消息后，网络环境发生变化或者波动，不能接收到第2，3，4条，这时候只发送第一条给serverA，导致性质判断出错】
  [在serverA端进行clientA的性质判断]
  从接收到第一条来自clientA的包含ASDFGH的消息起，在1分钟内，进行一下判断
  1.如果接收到了clientA的4条消息包中包含ASDFGH的消息，消息包中含有serverA的9930，9934，9935端口以及serverB的9930端口信息，可以判断clientA路由入时，对收到的所有信息进行转发
  2.如果接收到了clientA的3条消息包中包含ASDFGH的消息，消息包中含有serverA的9930，9934，9935端口信息，可以判断clientA路由入时，仅对相同的IP发来的信息进行转发
  3.如果接收到了clientA的1条消息包中包含ASDFGH的消息，消息包中仅有serverA的9930端口信息，可以判断clientA路由入时，仅对相同的IP且相同port发来的信息进行转发
  4.在1分钟结束后，如果是任意其他情况，可以判断为网络波动，重新对clientA的路由入性质进行判断，直到是以上3种情况
  记录下接收到的消息情况，记录形式可以是 
  --------------------------------------------------------------------------------
  clientA | in | type| serverA:9930 | serverA:9934 | serverA:9935 | serverB:9930 |
  --------------------------------------------------------------------------------
  记录完毕后，发送给其他server进行信息共享
  此时对clientA的路由性质判断完成(需要使用到的server是serverA和serverB)
  [clientB进行同样的操作]
  对clientA，clientB的信息收集完毕，在serverA端将收集到的信息表(如下)发送给clientA和clientB，开始clientA和clientB之间的打洞。
  ---------------------------------------------------------------
  clientA | out  | type| ip1:port1  |  ip2:port2  |  ip3:port3  |
  ---------------------------------------------------------------
  --------------------------------------------------------------------------------
  clientA | in | type| serverA:9930 | serverA:9934 | serverA:9935 | serverB:9930 |
  --------------------------------------------------------------------------------
  ---------------------------------------------------------------
  clientB | out  | type| ip1:port1  |  ip2:port2  |  ip3:port3  |
  ---------------------------------------------------------------
  --------------------------------------------------------------------------------
  clientB | in | type| serverA:9930 | serverA:9934 | serverA:9935 | serverB:9930 |
  --------------------------------------------------------------------------------
  对于clientB端的路由入性质，clientA端的路由出性质进行考虑
  1.如果clientB的路由入性质是对相同的IP发来的信息进行转发，或对收到的所有信息进行转发，则clientA直接对clientB进行扫描(相同ip下，clientB都能收到)，包的内容为key。
  2.如果clientB的路由入性质是仅对相同的IP且相同port发来的信息进行转，clientA的路由出性质是固定不变，则clientA直接对clientB进行扫描，包的内容为key。
  3.如果clientB的路由入性质是仅对相同的IP且相同port发来的信息进行转，clientA的路由出性质是按一定规律变化(port+1)，在相互扫描的情况下，如果clientA的25530端口发送了请求给clientB的21329端口，之后短时间内clientB的21329端口发送了请求给clientA的25530端口，此时clientA的端口已经发生变化，导致打洞失败。初步的想法是，clientA端放慢发送速度，使得端口变化缓慢，在端口没有发生变化的情况下，有尽可能多的请求从clientB发向clientA。并且可以合理利用clientA端路由出的变化规则，计算变化一次的时间T1，打洞经过的时间Ts，来判断clientA当前使用的端口，在clientB接收到clientA发来的消息后，向clientA当前使用的端口及其附近端口发送消息。包的内容都为key。
  4.如果clientB的路由入性质是仅对相同的IP且相同port发来的信息进行转，clientA的路由出性质是随机变化，这种情况目前初步的想法是clientA端放慢发送速度，使得端口变化缓慢，在端口没有发生变化的情况下，有尽可能多的请求从clientB发向clientA。包的内容都为key。
  (对于clientA端的路由入性质，clientB端的路由出性质进行同样考虑)
  
  如果打洞成功，clientA与clientB关闭打洞线程，转为发送心跳包的线程，并向serverA发送打洞成功的信号，在serverA端记录已经打洞成功的client和他们的端口号，记录形式可以是           
  -------------------------------------------
  clientA | ip:port | clientB | ip:port | key
  -------------------------------------------
  serverA在更新完毕打洞成功的client信息后，发送打洞表给其他server进行表的更新
  至此，初步环境建立
2.未知环境的clientX加入请求打洞
  clientX发送打洞请求给serverA，serverA接收到新打洞请求后，查看clientX是否已在打洞环境，如果已经有打洞成功的记录，返回打洞成功时的ip列表，作为clientX与现有列表中client打洞时使用的ip；如果无记录，则发送信号给需要打洞的clientA，clientB，以及接收到消息的回复给clientX。clientA,clientB接收到信号后同时进行以下操作。
  [例:与clientA进行两两之间的打洞]
  a.确定clientX的打洞ip
  clientX向serverA,serverB,serverC,serverD,serverE这5个不同环境服务器的9930端口发送请求，server之间共享消息，发送至serverA，serverA一共能收到5条消息，并且知道每条消息的【ip:port】，记录为
  -------------------------------------------------------------------------------
  clientX | A ip1:port1 | B ip2:port2 | C ip3:port3 | D ip4:port4 | E ip5:port5 |
  -------------------------------------------------------------------------------
  对这5条消息的ip进行讨论
  如果这5条消息的ip相同，则选取这个唯一的ip作为clientX的打洞ip
  如果这5条消息的ip不同，假设如下，则可以认为是以下情况，即对于clientX而言，有两层路由
  ----+----    IP：202.58.12.102   [3个]
      |        
  ----+----    IP：211.1.12.102    [2个]
      |
   clientX
  
  因为clientA是通用电信网下的客户端，因此serverA是clientA的内层路由。
  此时clientX如果要发送消息给clientA，在clientA端显示的是clientX发给serverA时显示的IP。
  选取serverA收到的clientX的ip作为clientX的打洞ip
  b.确定clientA的打洞ip
  clientX向serverA,serverB,serverC,serverD,serverE这5个不同环境服务器的9930端口发送请求，server之间共享消息，发送至serverA，serverA一共能收到5条消息，并且知道每条消息的【ip:port】，记录为
  -------------------------------------------------------------------------------
  clientA | A ip1:port1 | B ip2:port2 | C ip3:port3 | D ip4:port4 | E ip5:port5 |
  -------------------------------------------------------------------------------
  I.如果clientA的5条消息中的ip相同，结果显而易见，选取这个唯一ip作为clientA的打洞ip
  II.如果clientA的5条消息不同，且clienX的5条记录相同，则选取serverA接收到的clientA的ip作为其打洞ip
  III.如果clientA的5条消息不同，且clienX的5条记录也不同，而不同记录的服务器分布相同(如下图)，则可以认为clientX与clientA在相同的拓扑分布下,
  ----+----    IP：202.58.12.102   [serverC,serverD,serverE]
      |        
  ----+----    IP：211.1.12.102    [serverA,serverB]
      |
   clientX
  
  ----+----    IP：212.58.12.102   [serverC,serverD,serverE]
      |        
  ----+----    IP：213.1.12.102    [serverA,serverB]
      |
   clientA
  此时选取serverA接收到的clientA的ip作为其打洞ip
  IV.如果clientA的5条消息不同，clienX的5条记录也不同，不同记录的服务器分布也不同，但是clientA的某一层分布的server集合是clientX的某两层层分布的server集合之和(如下图)
  ----+----    IP：202.58.12.102   [serverD,serverE]
      |        
  ----+----    IP：214.1.12.102    [serverC]
      |        
  ----+----    IP：211.1.12.102    [serverA,serverB]
      |
   clientX
  
  ----+----    IP：212.58.12.102   [serverC,serverD,serverE]
      |
  ----+----    IP：213.1.12.102    [serverA,serverB]
      |
   clientA
  此时可以判断clientX与clientA在不同的拓扑分布下，选取不是serverA接收到的clientA的ip，作为其打洞ip（图中情况选取serverC,serverD,serverE接收到的A的ip）
  V.如果clientA的5条消息不同，clienX的5条记录也不同，不同记录的服务器分布也不同且无明显规律(如下图)
  ----+----    IP：202.58.12.102   [serverA,serverD,serverE]
      |        
  ----+----    IP：214.1.12.102    [serverB]
      |        
  ----+----    IP：211.1.12.102    [serverC]
      |
   clientX
  
  ----+----    IP：212.58.12.102   [serverC,serverD,serverE]
      |
  ----+----    IP：213.1.12.102    [serverA,serverB]
      |
   clientA
  则将clientA的若干个不同ip同时作为打洞ip
  serverA在判断过属于哪一种情况后，返回clientA的打洞ip给clientX，返回clientX的打洞ip给clientA，并且生成随机密钥给双方作为打洞信号
  clientA与clientX在接收到对方的打洞ip以及打洞信号后，进行同步骤1（环境初步判断）中的路由性质判断，进行打洞。
  如果打洞成功，clientA与clientX关闭打洞线程，转为发送心跳包的线程，并向serverA发送打洞成功的信号，在serverA端记录已经打洞成功的client和他们的端口号，记录形式可以是           
  -------------------------------------------
  clientA | ip:port | clientX | ip:port | key
  -------------------------------------------
  serverA在更新完毕打洞成功的client信息后，发送打洞表给其他server进行表的更新
  至此，clientX与clientA打洞完成。
3.未知环境的clientY与未知环境的clientX进行打洞
  clientX向serverA,serverB,serverC,serverD,serverE这5个不同环境服务器的9930端口发送请求，server之间共享消息，发送至serverA，serverA一共能收到5条消息，并且知道每条消息的【ip:port】，记录为
  -------------------------------------------------------------------------------
  clientX | A ip1:port1 | B ip2:port2 | C ip3:port3 | D ip4:port4 | E ip5:port5 |
  -------------------------------------------------------------------------------
  clientY进行同样的操作，记录为
  -------------------------------------------------------------------------------
  clientY | A ip1:port1 | B ip2:port2 | C ip3:port3 | D ip4:port4 | E ip5:port5 |
  -------------------------------------------------------------------------------
  如果有任意clientX或clientY记录中的server分布于clientA相同，则将这个client纳为通用电信网环境，同步骤2进行打洞，否则如下对clientX出现的全部ip返回给clientY,对clientY出现的全部ip返回给clientX，并且生成随机密钥给双方作为打洞信号
  clientX与clientY在接收到对方的打洞ip以及打洞信号后，进行同步骤1（环境初步判断）中的路由性质判断，进行打洞。
  如果打洞成功，clientX与clientY关闭打洞线程，转为发送心跳包的线程，并向serverA发送打洞成功的信号，在serverA端记录已经打洞成功的client和他们的端口号，记录形式可以是           
  -------------------------------------------
  clientX | ip:port | clientY | ip:port | key
  -------------------------------------------
  serverA在更新完毕打洞成功的client信息后，发送打洞表给其他server进行表的更新
  至此，clientX与clientY打洞完成。
  
  
  
  
  
  
  
  
  