package com.Alex.MouseT1.AutoTasks;


import android.app.Activity;
import android.os.Build;
import android.util.Log;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import com.Alex.MouseT1.OtherFun;

import org.json.JSONArray;
import org.json.JSONObject;


public class OneChatGPTHistory extends AppCompatActivity {
    @RequiresApi(api = Build.VERSION_CODES.TIRAMISU)
    public void startTask(Activity activity, String date, String postUrl) {
        try {

            String chatGPTHistories=OtherFun.readJsonFromFile(activity,"chatGPTHistory.json");
            JSONArray chatGPTHistoriesList=new JSONArray(chatGPTHistories);

            String finalResponse = "{\"type\":\"text\",\"text\":\"Chat not found\"}";
            for (int i = 0; i < chatGPTHistoriesList.length(); i++){
                JSONObject chatGPTHistory=chatGPTHistoriesList.getJSONObject(i);

                if (chatGPTHistory.getString("date").equals(date)){
                    finalResponse="{\"type\":\"text\",\"text\":\""+chatGPTHistory.getString("response")+"\"}";
                }
            }
            OtherFun.sendLargeJson(finalResponse,postUrl);
        } catch (Exception e) {
            Log.e("testaa", e.toString());
        }
    }
}