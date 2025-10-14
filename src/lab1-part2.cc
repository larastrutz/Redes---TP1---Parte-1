#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Lab1Part2");

int main(int argc, char* argv[])
{
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    uint32_t nCsma = 4; 
    uint32_t nPackets = 1;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
    cmd.AddValue("nPackets", "Number of packets to echo (1-20)", nPackets);
    cmd.Parse(argc, argv);

    if (nPackets > 20) nPackets = 20;

    NodeContainer p2pNodes1;
    p2pNodes1.Create(2);

    NodeContainer csmaNodes;
    csmaNodes.Add(p2pNodes1.Get(1)); 
    csmaNodes.Create(nCsma);        
    
    NodeContainer p2pNodes2;
    p2pNodes2.Add(csmaNodes.Get(nCsma)); 
    p2pNodes2.Create(1); 

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices1 = pointToPoint.Install(p2pNodes1);
    NetDeviceContainer p2pDevices2 = pointToPoint.Install(p2pNodes2);
    
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    NetDeviceContainer csmaDevices = csma.Install(csmaNodes);
    
    InternetStackHelper stack;
    stack.Install(p2pNodes1.Get(0));
    stack.Install(csmaNodes);
    stack.Install(p2pNodes2.Get(1)); 

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces1 = address.Assign(p2pDevices1);

    address.SetBase("10.1.2.0", "255.255.255.0");
    address.Assign(csmaDevices);

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces2 = address.Assign(p2pDevices2);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(p2pNodes2.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(25.0)); 

    UdpEchoClientHelper echoClient(p2pInterfaces2.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(nPackets));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(p2pNodes1.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(25.0)); 
    
    pointToPoint.EnablePcapAll("lab1-part2-p2p");
    csma.EnablePcapAll("lab1-part2-csma", false);

    Simulator::Stop(Seconds(25.0)); 
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
