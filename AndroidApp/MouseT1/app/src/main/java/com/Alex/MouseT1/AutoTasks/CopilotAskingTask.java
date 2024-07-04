package com.Alex.MouseT1.AutoTasks;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Instrumentation;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.SystemClock;
import android.util.Log;
import android.view.inputmethod.InputMethodManager;
import android.view.MotionEvent;
import android.view.View;
import android.webkit.ValueCallback;
import android.webkit.WebChromeClient;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import com.Alex.MouseT1.OtherFun;
import com.Alex.MouseT1.R;
import com.Alex.MouseT1.shell.CommandExecutionException;
import com.Alex.MouseT1.shell.CommandOutput;
import com.Alex.MouseT1.shell.Shell;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.file.Files;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Objects;

public class CopilotAskingTask extends AppCompatActivity {

    private boolean isPageLoaded = false;
    private String imageName = "copilot_image.png";
    private String postUrl;
    private String copilotAskingType;
    private WebView webView;

    @RequiresApi(api = Build.VERSION_CODES.O)
    @SuppressLint("SetJavaScriptEnabled")
    public void startTask(Activity activity, String imageSource, String askingType, String url) {
        try {
            if (!OtherFun.isNetworkConnected(activity)) {
                Log.e("CopilotAskingTask", "Not connected to internet");
                OtherFun.sendLargeJson("{\"type\":\"text\",\"text\":\"Erreur: Télephone non connecté à internet\"}", url);
            } else {

                OtherFun.unlockPhone(activity,postUrl);

                webView = activity.findViewById(R.id.webView);

                webView.setVisibility(View.VISIBLE);

                postUrl = url;
                copilotAskingType = askingType;

                String imagePath = getCaptureFromESP(imageSource, activity);
                setupWebView(activity, imagePath);
            }

        } catch (Exception e) {
            Log.e("testaa", e.toString());
        }
    }

    private String getCaptureFromESP(String imageSource,Activity activity) {
        try {
            URL url = new URL(imageSource);
            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            connection.setDoInput(true);
            connection.connect();
            InputStream input = connection.getInputStream();
            File storageDir = activity.getExternalFilesDir(null);
            File imageFile = new File(storageDir, imageName);
            FileOutputStream output = new FileOutputStream(imageFile);
            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = input.read(buffer)) != -1) {
                output.write(buffer, 0, bytesRead);
            }
            output.close();
            input.close();
            return imageFile.getAbsolutePath();
        } catch (Exception e) {
            Log.e("testaa", e.toString());
            return null;
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    @SuppressLint({"JavascriptInterface", "SetJavaScriptEnabled"})
    private void setupWebView(final Activity activity, String imagePath) throws InterruptedException {
        webView.getSettings().setJavaScriptEnabled(true);
        webView.getSettings().setAllowFileAccess(true);
        webView.getSettings().setAllowContentAccess(true);

        webView.addJavascriptInterface(new WebAppInterface(), "Android");

        webView.setWebChromeClient(new WebChromeClient() {
            @Override
            public boolean onShowFileChooser(WebView webView, ValueCallback<Uri[]> filePathCallback, FileChooserParams fileChooserParams) {
                String predefinedFilePath = copyFileFromInternalStorage(imagePath, activity);
                if (predefinedFilePath != null) {
                    filePathCallback.onReceiveValue(new Uri[]{Uri.fromFile(new File(predefinedFilePath))});
                } else {
                    launchFileChooser(activity);
                }
                return true;
            }
        });

        webView.setWebViewClient(new WebViewClient() {
            @Override
            public void onPageFinished(WebView view, String url) {
                try {
                    super.onPageFinished(view, url);

                    if (!isPageLoaded) {
                        isPageLoaded = true;
                        new AskingCopilotAndSending(activity).execute();
                    }
                } catch (Exception e) {
                    Log.e("testaa", e.toString());
                }
            }
        });
        OtherFun.sendLargeJson("{\"type\":\"text\",\"text\":\"Launching Copilot\"}", postUrl);
        webView.loadUrl("https://copilot.microsoft.com/");
    }

    @SuppressLint("StaticFieldLeak")
    public class AskingCopilotAndSending extends AsyncTask<Void, Void, String> {
        private final Activity mActivity;
        private String QUESTION = "";

        AskingCopilotAndSending(Activity activity) {
            this.mActivity = activity;
        }

        @SuppressLint("SimpleDateFormat")
        @RequiresApi(api = Build.VERSION_CODES.TIRAMISU)
        @Override
        protected String doInBackground(Void... voids) {
            try {

                switch (copilotAskingType) {
                    case "Réponse(courte) à la question":
                        QUESTION = "Repond moi correctement et rapidement a la question situee sur limage";
                        break;
                    case "Réponse(longue) à la question":
                        QUESTION = "Repond moi correctement et precisement a la question situee sur limage ";
                        break;
                    case "Rédaction":
                        QUESTION = "Fait moi une rédaction en français sur ce que demande limage";
                        break;
                    case "Traduction(->Français)":
                        QUESTION = "Traduis moi vers le français le texte sur limage";
                        break;
                    case "Traduction(->Anglais)":
                        QUESTION = "Traduis moi en vers l'anglais le texte sur limage";
                        break;
                    case "Traduction(->Espagnol)":
                        QUESTION = "Traduis moi en vers l'espagnol le texte sur limage";
                        break;
                    case "Résolution d'équation":
                        QUESTION = "Resoue moi bien léquetion sur limage, je ne veux pas derreur";
                        break;
                    case "Ortographe d'un mot":
                        QUESTION = "Dit moi comment secrit le mot ecrit sur image";
                        break;
                    case "Définition d'un mot":
                        QUESTION = "Donne moi la définition du texte sur limage";
                        break;
                    case "Trouver la date":
                        QUESTION = "Trouve moi la date de l'événement decrit sur limage";
                        break;
                    case "Description d'image":
                        QUESTION = "Decris moi precisement limage donne";
                        break;
                }

                // Navigation
                Thread.sleep(1000);
                simulateClick(100, 100);//close popup
                Thread.sleep(500);
                simulateClick(550, 1320);//give image
                Thread.sleep(1200);
                simulateClick(500, 1100);//focus input text
                Thread.sleep(1200);

                String[] words = QUESTION.split("\\s+");

                for (String word : words) {
                    Thread.sleep(100);
                    executeRootCommand("input text " + word);
                    Thread.sleep(100);
                    executeRootCommand("input tap 500 1370"); // space
                }
                executeRootCommand("input tap 630 1300"); // backspace
                Thread.sleep(100);
                executeRootCommand("input tap 630 1370"); // enter
                Thread.sleep(1000);

                OtherFun.sendLargeJson("{\"type\":\"text\",\"text\":\"Generating response...\"}", postUrl);

                InputMethodManager imm = (InputMethodManager) mActivity.getSystemService(Context.INPUT_METHOD_SERVICE);
                while (!imm.isAcceptingText()) {
                    Thread.sleep(1000);
                    imm = (InputMethodManager) mActivity.getSystemService(Context.INPUT_METHOD_SERVICE);
                    simulateClick(1, 1);
                }

                Thread.sleep(2500);
                simulateClick(221, 530);
                Thread.sleep(1200);

                ClipboardManager clipboard = (ClipboardManager) mActivity.getSystemService(Context.CLIPBOARD_SERVICE);
                String response = clipboard.getText().toString();
                int responsePosition = response.indexOf("Source : conversation avec Copilot");
                String filteredResponse = response.substring(0, responsePosition).replaceAll("\n+$", "");
                String finalResponse = "{\"type\":\"text\",\"text\":\"" + filteredResponse + "\"}";

                // Sending to ESP32
                OtherFun.sendLargeJson(finalResponse, postUrl);

                // Saving to history
                JSONArray chatGPTHistoriesList = new JSONArray("[]");

                JSONObject chatGPTObject = new JSONObject();
                chatGPTObject.put("Asking type", copilotAskingType);
                chatGPTObject.put("date", new SimpleDateFormat("MMMM dd HH:mm:ss yyyy").format(new Date()));
                chatGPTObject.put("response", filteredResponse);

                chatGPTHistoriesList.put(chatGPTObject);

                OtherFun.writeJsonToFile(mActivity, "chatGPTHistory.json", chatGPTHistoriesList.toString());

            } catch (Exception e) {
                Log.e("testaa", "Error in doInBackground: " + e.getMessage());
            }
            return null;
        }

        @Override
        protected void onPostExecute(String result) {
            super.onPostExecute(result);
            if (result != null) {
                Log.d("CopilotAskingTask", "Response: " + result);
            }
        }
    }

    private void executeRootCommand(String command) {
        try {
            Shell shell = new Shell();
            CommandOutput output = shell.executeRootCommand(command);
            if (!output.toString().isEmpty()) {
                Log.d("CopilotAskingTask", "Output: " + output + " Command: " + command);
            }
        } catch (CommandExecutionException e) {
            throw new RuntimeException(e);
        }
    }

    private void simulateClick(int x, int y) {
        try {
            Instrumentation inst = new Instrumentation();
            long downTime = SystemClock.uptimeMillis();
            long eventTime = SystemClock.uptimeMillis();
            MotionEvent downEvent = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_DOWN, x, y, 0);
            MotionEvent upEvent = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_UP, x, y, 0);
            inst.sendPointerSync(downEvent);
            inst.sendPointerSync(upEvent);
        } catch (Exception e) {
            if (!(Objects.requireNonNull(e.getMessage()).contains("Injecting to another application requires INJECT_EVENTS permission"))) {
                Log.e("CopilotAskingTask", e.toString());
            }
        }
    }

    private void launchFileChooser(Activity activity) {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");
        activity.startActivityForResult(Intent.createChooser(intent, "File Chooser"), 1);
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    private String copyFileFromInternalStorage(String filePath, Activity activity) {
        File cacheFile = new File(activity.getCacheDir(), new File(filePath).getName());
        try (InputStream inputStream = Files.newInputStream(new File(filePath).toPath());
             FileOutputStream outputStream = new FileOutputStream(cacheFile)) {
            byte[] buffer = new byte[1024];
            int length;
            while ((length = inputStream.read(buffer)) > 0) {
                outputStream.write(buffer, 0, length);
            }
            return cacheFile.getAbsolutePath();
        } catch (IOException e) {
            Log.e("CopilotAskingTask", "Error copying file from internal storage", e);
            return null;
        }
    }

    public static class WebAppInterface {
        WebAppInterface() {
        }
    }

}
