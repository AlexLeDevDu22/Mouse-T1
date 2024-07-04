package com.Alex.MouseT1

import android.annotation.SuppressLint
import android.content.Intent
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.os.StrictMode
import android.util.Log
import android.view.View.GONE
import android.webkit.WebView
import android.widget.TextView
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AppCompatActivity
import com.Alex.MouseT1.AutoTasks.CopilotAskingTask
import com.Alex.MouseT1.AutoTasks.OneChatGPTHistory
import com.Alex.MouseT1.AutoTasks.OneLessonsTask
import com.Alex.MouseT1.AutoTasks.OverviewChatGPTHistory
import com.Alex.MouseT1.AutoTasks.OverviewLessonsTask
import com.Alex.MouseT1.UI.LessonManagerUI
import com.Alex.MouseT1.databinding.ActivityMainBinding
import com.Alex.MouseT1.shell.Shell
import org.json.JSONObject
import java.io.BufferedReader
import java.net.HttpURLConnection
import java.net.URL

class MainActivity : AppCompatActivity() {
    private lateinit var binding: ActivityMainBinding
    private lateinit var handler: Handler

    private var lastRequestDid: JSONObject? = null
    private var taskThread: Thread? = null

    private lateinit var CopilotAskingTask: CopilotAskingTask
    private lateinit var OneChatGPTHistory: OneChatGPTHistory
    private lateinit var OneLessonsTask: OneLessonsTask
    private lateinit var OverviewChatGPTHistory: OverviewChatGPTHistory
    private lateinit var OverviewLessonsTask: OverviewLessonsTask

    @RequiresApi(api = Build.VERSION_CODES.TIRAMISU)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(R.layout.activity_main)


        val gfgPolicy = StrictMode.ThreadPolicy.Builder().permitAll().build()
        StrictMode.setThreadPolicy(gfgPolicy)//allow Network on main Thread


        //init class
        CopilotAskingTask=CopilotAskingTask()
        OneChatGPTHistory=OneChatGPTHistory()
        OneLessonsTask= OneLessonsTask()
        OverviewChatGPTHistory= OverviewChatGPTHistory()
        OverviewLessonsTask= OverviewLessonsTask()

        //start request listener loop
        handler = Handler(Looper.getMainLooper())
        handler.post(object : Runnable {
            override fun run() {
                waitingRequest()
                handler.postDelayed(this, 400)
            }
        })

        binding.buttonManageLesson.setOnClickListener {
            val intent = Intent(this, LessonManagerUI::class.java)
            startActivity(intent)
        }
    }

    @SuppressLint("SetTextI18n")
    @RequiresApi(api = Build.VERSION_CODES.TIRAMISU)
    private fun waitingRequest(){

        var jsonResponse: JSONObject? = null
        val esp32ConnectedFlag: TextView = findViewById(R.id.esp32connectedFlag)
        try {
            val ip = getESP32IPAddress()
            if (ip != "Error: ESP32 not found") {
                val url = URL("http://$ip/json-get")
                val connection = url.openConnection() as HttpURLConnection
                connection.requestMethod = "GET"
                connection.connectTimeout = 3000
                connection.readTimeout = 3000

                val responseCode = connection.responseCode
                if (responseCode == HttpURLConnection.HTTP_OK) {
                    val inputStream = connection.inputStream
                    val response = inputStream.bufferedReader().use(BufferedReader::readText)
                    jsonResponse = JSONObject(response)
                    esp32ConnectedFlag.post {
                        esp32ConnectedFlag.text = "ESP32 connected!"
                    }
                } else {
                    Log.e("testaa", "Failed to connect to ESP32: $responseCode")
                    esp32ConnectedFlag.post {
                        esp32ConnectedFlag.text = "ESP32 not connected"
                    }
                }
                connection.disconnect()
            } else {
                Log.e("testaa", "ESP32 not connected")
                esp32ConnectedFlag.post {
                    esp32ConnectedFlag.text = "ESP32 not connected"
                }
            }
        } catch (e: Exception) {
            Log.e("testaa", "Error in waitingRequest: $e")
            esp32ConnectedFlag.post {
                esp32ConnectedFlag.text = "ESP32 not connected"
            }
        }

        if (jsonResponse.toString() != lastRequestDid?.toString() && jsonResponse!=null) {
            Log.d("testaa", "new request: $jsonResponse")
            lastRequestDid = jsonResponse

            handleRequest(jsonResponse)
        }
    }

    @SuppressLint("CutPasteId")
    @RequiresApi(api = Build.VERSION_CODES.TIRAMISU)
    private fun handleRequest(requestInput: JSONObject) {
        val esp32ConnectedFlag: TextView = findViewById(R.id.esp32connectedFlag)

        //destroy all Threads
        findViewById<WebView>(R.id.webView).visibility=GONE
        taskThread?.interrupt()


        esp32ConnectedFlag.post {
            try {
                if (requestInput.getString("type") != "no request") {
                    val postUrl=requestInput.getString("post url")

                    taskThread = Thread {
                        this.runOnUiThread {
                            when (requestInput.getString("type")) {
                                "chatGPT asking" -> {
                                    CopilotAskingTask.startTask(
                                        this@MainActivity,
                                        requestInput.getString("image source"),
                                        requestInput.getString("asking type"),
                                        postUrl
                                    )
                                }
                                "overview lessons" -> {
                                    OverviewLessonsTask.startTask(
                                        this@MainActivity,
                                        requestInput.getString("matiere"),
                                        postUrl
                                    )
                                }
                                "one lesson" -> {
                                    OneLessonsTask.startTask(
                                        this@MainActivity,
                                        requestInput.getString("lesson title"),
                                        postUrl
                                    )
                                }
                                "chatGPT history overview" -> {
                                    OverviewChatGPTHistory.startTask(
                                        this@MainActivity,
                                        requestInput.getString("asking type"),
                                        postUrl
                                    )
                                }
                                "one chatGPT history" -> {
                                    OneChatGPTHistory.startTask(
                                        this@MainActivity,
                                        requestInput.getString("date"),
                                        postUrl
                                    )
                                }
                            }

                        }
                    }
                    taskThread!!.start()
                    OtherFun.sendLargeJson("{'type':'text','text':'Request received, starting task'}", postUrl)
                }
            } catch (e: Exception) {
                Log.e("testaa", "Error handling request: $e")
            }
        }
    }


    private fun getESP32IPAddress(): String {
        return try {
            val shell = Shell()
            val arpResult = shell.executeRootCommand("cat /proc/net/arp").toString()
            var ipAddress = "Error: ESP32 not found"
            arpResult.lines().forEach { line ->
                val columns = line.split("\\s+".toRegex())
                if (columns.size >= 4 && columns[3] == "ec:64:c9:aa:96:ec" && columns[2] == "0x2") {
                    ipAddress = columns[0]
                }
            }
            ipAddress
        } catch (e: Exception) {
            Log.e("testaa", "Error in getESP32IPAddress: $e")
            "Error: ESP32 not found"
        }
    }
}