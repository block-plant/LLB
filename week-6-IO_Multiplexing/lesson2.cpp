// To solve the C10K problem, we must let the macOS kernel 
// do the heavy lifting. Instead of asking 10,000 times if 
// data is ready, we hand the kernel a list of file 
// descriptors and say: "Put my thread to sleep. 
// Wake me up only when one of these has data."

// today the topic is : The kqueue Event Monitor