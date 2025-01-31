#!/usr/bin/env python3
import json
import os
from rich import print

# x = """
# "{
#  "data": {
#   "1690910904000": "{\"AppCount\":{\"ApplicationCount\":14.0,\"ApplicationStatusDetail\":[{\"applicationId\":\"67d22f4cb1a043bdb9f42d4928aeb146\",\"title\":\"Notifier\",\"appStatus\":\"running\",\"repoName\":\"notifier109\"},{\"applicationId\":\"c21fbcdef2a84eda8a4dd71b8b1e742f\",\"title\":\"External Databus\",\"appStatus\":\"running\",\"repoName\":\"iemqttc\"},{\"applicationId\":\"de70bbfed431455eb4c5404fe4afdd03\",\"title\":\"Data Service\",\"appStatus\":\"running\",\"repoName\":\"dataservice109\"},{\"applicationId\":\"1dbf9af1b4114390a3b63712551e674b\",\"title\":\"IE Flow Creator\",\"appStatus\":\"running\",\"repoName\":\"ieflowcreator\"},{\"applicationId\":\"d0e61067a4ad41aa9381b6c38774ec72\",\"title\":\"Databus\",\"appStatus\":\"running\",\"repoName\":\"db\"},{\"applicationId\":\"b3de9fcf914240f48858f8b3209374c5\",\"title\":\"SIMATIC S7 Connector\",\"appStatus\":\"running\",\"repoName\":\"sa\"},{\"applicationId\":\"ad70b6b02b62412d92007fafa294c7da\",\"title\":\"Data Flow Monitoring\",\"appStatus\":\"running\",\"repoName\":\"dfmrt\"},{\"applicationId\":\"f1227bd9f0fe46dba4c2b93f39ea8fa2\",\"title\":\"Alerta\",\"appStatus\":\"running\",\"repoName\":\"alerta\"},{\"applicationId\":\"3d8e1f8327d146228e094e63ac23032d\",\"title\":\"Heptapod-Explorer\",\"appStatus\":\"running\",\"repoName\":\"heptapodexplorer\"},{\"applicationId\":\"b4502ee6f63f070e42610321a47e29d1\",\"title\":\"IIH Core\",\"appStatus\":\"running\",\"repoName\":\"iihcore\"},{\"applicationId\":\"t8ajyaZ3LjvbLG5dlfdrppeFFgvXgJ5y\",\"title\":\"Registry Service\",\"appStatus\":\"running\",\"repoName\":\"cssvcregistry\"},{\"applicationId\":\"t88jyaZ3LjvbLG5dlfdrppeFFgvXgJ5y\",\"title\":\"Databus Gateway\",\"appStatus\":\"running\",\"repoName\":\"csmqttgw\"},{\"applicationId\":\"c3336bcc3bb44114f209d7e8c8aefcc5\",\"title\":\"IIH Configurator\",\"appStatus\":\"running\",\"repoName\":\"iihconfigurator\"},{\"applicationId\":\"0b3eb768aea749a295458155dde018f3\",\"title\":\"idapp\",\"appStatus\":\"running\",\"repoName\":\"idapp\"}],\"MaxInstalledApp\":20.0,\"MaxRunningApp\":10.0,\"RunningApplicationsCount\":14.0},\"DeviceId\":\"c40df90283a246aa8da39ce17affc4b1\",\"InternetStatus\":{\"serialVersionUID\":0.0,\"NetworkType\":\"RelayNR\",\"RouterIP\":\"\",\"RelayServerID\":\"\",\"RelayServerAddress\":\"\",\"CurrentConnectedIP\":\"\",\"CurrentConnectedPort\":\"\",\"currentConnectedKeystonePort\":\"\",\"RelayServerUsername\":\"\",\"RelayServerPassword\":\"\",\"RelayServerPort\":\"\",\"RURLAddress\":\"\",\"isRelaySet\":false,\"hostName\":\"\",\"isRURLSet\":false,\"LanIPAddresses\":[\"10.241.124.49\"],\"ExternalIPaddress\":[\"Kraken.10.241.124.24:9443\",\"10.241.124.49\"],\"BoxID\":\"4868f6e1d5944222b1a5e71c3a699e65\",\"TomcatPort\":\"443\",\"RURLTomcatPort\":\"\"},\"OperationalQualities\":{\"maxInstalledAppCount\":20.0,\"maxRunningAppCount\":10.0,\"maxUsableRamUsage\":30576.64,\"maxUsableStorageArea\":55398.4,\"maxCPULoad\":100.0},\"StorageInfo\":{\"DataSpaceFree\":\"25.3 GB\",\"DataSpacePercentage\":\"47.99%\",\"DataSpaceTotal\":\"54.1 GB\",\"DataSpaceUsed\":\"26 GB\"},\"SystemInfo\":{\"CPUInfo\":\"x86_64\",\"CpuCores\":\"4\",\"CpuPercentage\":\"9.55\",\"GetUptime\":\"3 days, 23 hours, 6 minutes\",\"KubeAppliance\":\"false\",\"MemoryFree\":\"27.3 GB\",\"MemoryPercentage\":\"11.68\",\"MemoryTotal\":\"31.4 GB\",\"MemoryUsed\":\"3.7 GB\"},\"correlationID\":\"dad447bd996a-acdc0769-f061-4f19-8d09-adcb3f5567e3\",\"currenttime\":1.690910903048E12,\"devicesCount\":0.0,\"interface\":[{\"interfaceName\":\"ens160\",\"macAddress\":\"00:0C:29:76:7B:71\",\"iPAddress\":\"10.241.124.49\",\"subNetMask\":\"255.255.254.0\",\"status\":\"Enabled\",\"gateway\":\"10.241.124.1\",\"primaryDns\":\"10.241.124.1\",\"secondaryDns\":\"\",\"hostName\":\"kraken\",\"domainName\":\"kraken.10.241.124.24:9443\",\"isDHCP\":true,\"isEnabled\":true,\"dockerIP\":\"172.17.0.0/16\",\"gatewayInterface\":false}],\"ntpstatus\":{\"lastSyncTime\":1.690910902947E12,\"status\":\"NOT_SYNC\"}}"
#  }
# }"
# """

# Here, x_example.json is the direct output from the iectl command:
# $EDGE_SKIP_TLS=1 iectl iem device get-statistics --deviceId c40df90283a246aa8da39ce17affc4b1 > x_example.json 
x = json.load(open('x_example.json', 'r'))

# Adapted from recursive map through complex json from 
# https://stackoverflow.com/a/12507546
def dict_generator(indict, pre=None):
    pre = pre[:] if pre else []
    if isinstance(indict, dict):
        for key, value in indict.items():
            if isinstance(value, dict):
                for d in dict_generator(value, pre + [key]):
                    yield d
            elif isinstance(value, list) or isinstance(value, tuple):
                for v in value:
                    for d in dict_generator(v, pre + [key]):
                        yield d
            elif isinstance(value, str) and len(value) > 0 and (value[0] == '{' or value[0] =='['):
                value = json.loads(value)
                for d in dict_generator(value, pre + [key]):
                    yield d
            else:
                yield pre + [key, value]
    else:
        yield pre + [indict] 


def main():
    data = list(dict_generator(x))
    print(data)

    for datum in data:        
        with open(f"/tmp/zdata/{datum[-2]}", "w") as f:
            f.write(f"{datum[-1]}")



if __name__ == "__main__":
    main()
