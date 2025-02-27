// Copyright 2019 The Bazel Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

import Foundation
import SwiftProtobuf
import GRPC
import NIOCore
import NIOPosix
import examples_xplatform_grpc_echo_proto
import examples_xplatform_grpc_echo_client_services_swift

// Setup an `EventLoopGroup` for the connection to run on.
//
// See: https://github.com/apple/swift-nio#eventloops-and-eventloopgroups
let group = MultiThreadedEventLoopGroup(numberOfThreads: 1)

// Make sure the group is shutdown when we're done with it.
defer {
  try! group.syncShutdownGracefully()
}

// Configure the channel, we're not using TLS so the connection is `insecure`.
let channel = try GRPCChannelPool.with(
  target: .host("localhost", port: 9000),
  transportSecurity: .plaintext,
  eventLoopGroup: group
)

// Initialize the client using the same address the server is started on.
let client = RulesSwift_Examples_Grpc_EchoServiceClient(channel: channel)

// Construct a request to the echo service.
var request = RulesSwift_Examples_Grpc_EchoRequest.with {
  $0.contents = "Hello, world!"
  let timestamp = Google_Protobuf_Timestamp(date: Date())
  request.extra = try! Google_Protobuf_Any(message: timestamp)
}

let call = client.echo(request)

// Make the remote method call and print the response we receive.
do {
  let response = try call.response.wait()
  print(response.contents)
} catch {
  print("Echo failed: \(error)")
}
