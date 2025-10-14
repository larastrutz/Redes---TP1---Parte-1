#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Lab1Part1");

int main(int argc, char* argv[])
{
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    uint32_t nClients = 1;
    uint32_t nPackets = 1;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nClients", "Number of client nodes (1-5)", nClients);
    cmd.AddValue("nPackets", "Number of packets per client (1-5)", nPackets);
    cmd.Parse(argc, argv);

    if (nClients > 5) nClients = 5;
    if (nPackets > 5) nPackets = 5;

    NodeContainer serverNode;
    serverNode.Create(1); 

    NodeContainer clientNodes;
    clientNodes.Create(nClients);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    InternetStackHelper stack;
    stack.Install(serverNode);
    stack.Install(clientNodes);

    Ipv4AddressHelper address;
    Ipv4InterfaceContainer serverInterfaces; 
    
    for (uint32_t i = 0; i < nClients; ++i)
    {
        NodeContainer nodes = NodeContainer(serverNode.Get(0), clientNodes.Get(i));
        NetDeviceContainer devices = pointToPoint.Install(nodes);

        std::string subnet = "10.1." + std::to_string(i + 1) + ".0";
        address.SetBase(subnet.c_str(), "255.255.255.0");
        Ipv4InterfaceContainer interfaces = address.Assign(devices);

        serverInterfaces.Add(interfaces.Get(0));
    }
    
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    UdpEchoServerHelper echoServer(15);
    ApplicationContainer serverApps = echoServer.Install(serverNode.Get(0));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(20.0)); 

    Ipv4Address serverAddress = serverInterfaces.GetAddress(0);

    for (uint32_t i = 0; i < nClients; ++i)
    {
        UdpEchoClientHelper echoClient(serverAddress, 15);
        echoClient.SetAttribute("MaxPackets", UintegerValue(nPackets));
        echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        echoClient.SetAttribute("PacketSize", UintegerValue(1024));

        ApplicationContainer clientApps = echoClient.Install(clientNodes.Get(i));

        Ptr<UniformRandomVariable> startTime = CreateObject<UniformRandomVariable>();
        startTime->SetAttribute("Min", DoubleValue(2.0));
        startTime->SetAttribute("Max", DoubleValue(7.0));
        
        clientApps.Start(Seconds(startTime->GetValue()));
        clientApps.Stop(Seconds(20.0)); 
    }

    Simulator::Stop(Seconds(20.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
