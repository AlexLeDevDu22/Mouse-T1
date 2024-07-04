package com.Alex.MouseT1.AutoTasks;

import android.app.Activity;
import android.os.Build;
import android.util.Log;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import com.Alex.MouseT1.OtherFun;

import org.json.JSONArray;
import org.json.JSONObject;

import java.time.Duration;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.Locale;

public class OverviewChatGPTHistory extends AppCompatActivity {
    @RequiresApi(api = Build.VERSION_CODES.TIRAMISU)
    public void startTask(Activity activity, String askingType, String postUrl) {
        try {

            String chatGPTHistories = OtherFun.readJsonFromFile(activity, "chatGPTHistory.json");


            JSONArray chatGPTHistoriesList = new JSONArray(chatGPTHistories);

            JSONArray lessonToSend = new JSONArray();
            for (int i = 0; i < chatGPTHistoriesList.length(); i++) {
                JSONObject chatGPTHistory = chatGPTHistoriesList.getJSONObject(i);

                if (chatGPTHistory.getString("Asking type").equals(askingType)) {
                    String date=getElapsedTime(chatGPTHistory.getString("date"));
                    lessonToSend.put("{'text':'"+date+"', 'date':'"+chatGPTHistory.getString("date")+"'}");
                }
            }

            String finalResponse;
            if (lessonToSend.length() > 0) {
                finalResponse = "{\"type\":\"overview\",\"response\":\"chatGPT history overview\",\"overview\":" + lessonToSend.toString().replace("\"","") + "}";
            } else {
                finalResponse = "{\"type\":\"text\",\"text\":\"No chatGPT history found\"}";
            }

            OtherFun.sendLargeJson(finalResponse, postUrl);
            Log.d("testaa",finalResponse);
        } catch (Exception e) {
            Log.e("testaa", e.toString());
        }
    }
    @RequiresApi(api = Build.VERSION_CODES.O)
    public static String getElapsedTime(String dateString) {
        // Define the formatter to parse the input string
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("MMMM dd HH:mm:ss yyyy",Locale.FRENCH);

        // Parse the input string to a LocalDateTime object
        LocalDateTime inputTime = LocalDateTime.parse(dateString, formatter);

        // Get the current time
        LocalDateTime now = LocalDateTime.now();

        // Calculate the duration between the input time and now
        Duration duration = Duration.between(inputTime, now);

        // Calculate the difference in days, hours, and minutes
        long days = duration.toDays();
        long hours = duration.toHours() % 24;
        long minutes = duration.toMinutes() % 60;

        // Build the output string
        StringBuilder elapsedTime = new StringBuilder();
        if (days > 0) {
            elapsedTime.append(days).append("j ");
        }
        if (hours > 0 || days > 0) {
            elapsedTime.append(hours).append("h ");
        }
        elapsedTime.append(minutes).append("min");

        return elapsedTime.toString().trim();
    }
}
