/* @@@LICENSE
*
* Copyright (c) 2013 LG Electronics, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */

var Connection = require('ssh2');
var keypress = require('keypress'), tty = require('tty');
keypress(process.stdin);
var fs = require('fs');
var data = fs.readFileSync('./config.json'), connectionConfig;
var c = new Connection();
var shardId;

  try {
    connectionConfig = JSON.parse(data);
    console.dir(connectionConfig);
  }
  catch (err) {
    console.log('There has been an error parsing your JSON.')
    console.log(err);
  }

c.on('ready', function() {
  console.log('Connection :: connect');
  runCases([
    //1 PutKind
    lunaCase("luna-send -n 1 -a com.palm.contacts luna://com.palm.db/putKind '{\"id\":\"com.palm.test:1\",\"owner\":\"com.palm.contacts\",\"indexes\":[{\"name\":\"foo\", \"props\":[{\"name\":\"foo\"}]},{\"name\":\"barfoo\",\"props\":[{\"name\":\"bar\"},{\"name\":\"foo\"}]}]}]}'", function(id, result, finalize) {
      if (result.returnValue == true) {
        console.log("Return Value: " + result.returnValue)
      }
      finalize();
    }),
    //2 Put nonShard record
    lunaCase("luna-send -n 1 -a com.palm.contacts luna://com.palm.db/put '{\"objects\":[{\"_kind\":\"com.palm.test:1\",\"foo\":1,\"bar\":1000}]}'", function(id, result, finalize){
      if (result.returnValue == true) {
        console.log("Return Value: " + result.returnValue);
        console.log("Please insert USB Stick and press ENTER" )
        process.stdin.resume();
        process.stdin.once('keypress', function (ch, key) {
        if (key.name == 'enter') {
            process.stdin.pause();
            finalize();

        }
        });
      }
    }),
    //3 ListActiveMedia
    lunaCase("luna-send -n 1 luna://com.palm.db/listActiveMedia '{}'", function(id, result, finalize){
      if ( result.returnValue == true ) {
        console.log("Return Value: " + result.returnValue);
        shardId = result.media[0].shardId;
        console.log("ShardID: " + shardId)
        finalize();
      }
    }),
    //4 ShardInfo
    lunaCase(function() { return "luna-send -f -n 1  luna://com.palm.db/shardInfo '{\"shardId\": \""+ shardId + "\"}'"; }, function(id, result, finalize){
      if (result.returnValue == true) {
        console.log("Return Value: " + result.returnValue);
      }
      finalize();
    }),
    //5 Put Shard Records
    lunaCase(function() { return "luna-send -n 1 -a com.palm.contacts luna://com.palm.db/put '{\"shardId\":\"" + shardId + "\",\"objects\":[{\"_kind\":\"com.palm.test:1\",\"foo\":2,\"bar\":2000}]}'"; }, function(id, result, finalize){
      if (result.returnValue == true) {
        console.log("Return Value: " + result.returnValue);
      }
      finalize();
    }),
    //6 All Records
    lunaCase("luna-send -n 1 -a com.palm.contacts luna://com.palm.db/search '{\"query\":{\"from\":\"com.palm.test:1\"}}'", function(id, result, finalize){
      if (result.returnValue == true) {
        console.log("Return Value: " + result.returnValue);
        console.log("Please remove USB Stick and press ENTER" )
        process.stdin.once('keypress', function (ch, key) {
            if (key.name == 'enter') {
                process.stdin.pause();
                finalize();
            }
        });
        process.stdin.resume();
      }
    }),
    //7 ShardInfo
    lunaCase(function() { return "luna-send -f -n 1  luna://com.palm.db/shardInfo '{\"shardId\": \""+ shardId + "\"}'"; }, function(id, result, finalize){
      if (result.returnValue == true) {
        console.log("Return Value: " + result.returnValue);
      }
      finalize();
    }),
    //8 AllRecords
    lunaCase("luna-send -n 1 -a com.palm.contacts luna://com.palm.db/search '{\"query\":{\"from\":\"com.palm.test:1\"}}'", function(id, result, finalize){
      if (result.returnValue == true) {
        console.log("Return Value: " + result.returnValue);
      }
      finalize();
    }),
    //9 Mark Shard as transient
    lunaCase(function() { return "luna-send -f -n 1  luna://com.palm.db/setShardMode '{\"shardId\": \""+ shardId +"\", \"transient\":true}'"; }, function(id, result, finalize){
      if (result.returnValue == true) {
        console.log("Return Value: " + result.returnValue);
      }
      finalize();
    }),
    //10 Shard Info
    lunaCase(function() { return "luna-send -f -n 1  luna://com.palm.db/shardInfo '{\"shardId\": \""+ shardId + "\"}'"; }, function(id, result, finalize){
      if (result.returnValue == true) {
        console.log("Return Value: " + result.returnValue);
      }
      finalize();
    }),
    //11 Purge Records
    lunaCase("date 102012002013 &>/dev/null && luna-send -f -n 1  -a com.palm.spacecadet luna://com.palm.db/purge '{\"window\":2}'", function(id, result, finalize){
      if (result.returnValue == true) {
        console.log("Return Value: " + result.returnValue);
        console.log("Please re-Insert USB Stick and press ENTER" )
        process.stdin.resume();
        process.stdin.once('keypress', function (ch, key) {
        if (key.name == 'enter') {
            process.stdin.pause();
            finalize();
        }
        });
      }
    }),
    //12 All Records
    lunaCase("date 112012002013 &>/dev/null && luna-send -n 1 -a com.palm.contacts luna://com.palm.db/search '{\"query\":{\"from\":\"com.palm.test:1\"}}'", function(id, result, finalize){
      if (result.returnValue == true) {
        console.log("Return Value: " + result.returnValue);
      }
      finalize();
    }),
    //13 Delete kind
    lunaCase("luna-send -n 1 -a com.palm.contacts luna://com.palm.db/delKind '{\"id\":\"com.palm.test:1\"}'", function (id, result, finalize){
        if (result.returnValue == true) {
        console.log("Return Value: " + result.returnValue);
        }
        finalize();
    }),
    //14 didnt revert to not transient.
    lunaCase(function() { return "luna-send -f -n 1  luna://com.palm.db/setShardMode '{\"shardId\": \""+ shardId +"\", \"transient\":false}'"; }, function (id, result, finalize){
        finalize();
    })
      ]);
  });

  // command: string to execute
// caseCallback: function that takes (id, jsonResult, finalizeCallback)
function lunaCase(command, caseCallback) {
  return function(id, finalize) {
    console.log("\nrunning case #" + id)
    if (typeof command === "function") {
        command = command();
    }
      console.log("executing: " + command);
      c.exec(command, function(err, stream) {
        if (err) throw err; // XXX: who catches?

        var content = "";

        stream.on('data', function(block) {
          content += block;
        });

        stream.on('close', function() {
          console.log("OUT: " + content);
          var result = JSON.parse(content);
          returnValue = result.returnValue;
          if (!returnValue) {
            console.log("ERROR: returnValue - FALSE")
            process.abort();
          }

          caseCallback(id, result, finalize);
        });
      });
  }
}

// cases: functions that accepts (id, finalizeCallback)
function runCases(cases /*, callback*/) {
  console.log("cases: " + cases.length)
  var nextCase = function(id) {
    //console.log("cases: " + cases)
    if (id == cases.length) {
      //callback();
      process.exit();
    }
    cases[id](id, function() {
        nextCase(id+1);
    });
  }
  nextCase(0);
}

c.on('end', function(){
});

c.on('close', function(had_error) {
  console.log('Connection :: close');

});

c.connect(connectionConfig);
