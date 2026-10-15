package io.zoompilot.jetlink

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.view.WindowManager
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.core.content.ContextCompat
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.lifecycleScope
import androidx.lifecycle.repeatOnLifecycle
import io.zoompilot.jetlink.server.ServerService
import io.zoompilot.jetlink.ui.JetlinkTheme
import io.zoompilot.jetlink.ui.RootScreen
import kotlinx.coroutines.launch

/**
 * The dashboard. Opening it starts the server, which then runs in its
 * foreground service until stopped from the notification.
 *
 * Launch extras, for benches and screenshots as on the iPhone:
 * `adb shell am start -n io.zoompilot.jetlink.android/io.zoompilot.jetlink.MainActivity -e tab models`
 * opens a tab (status, models, benchmark, settings, logs, connect), and
 * `--ei benchmark 60` runs a benchmark of that many seconds.
 */
class MainActivity : ComponentActivity() {
    private val notifications = registerForActivityResult(ActivityResultContracts.RequestPermission()) {}

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        ServerService.start(this)
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED) {
            notifications.launch(Manifest.permission.POST_NOTIFICATIONS)
        }
        lifecycleScope.launch {
            repeatOnLifecycle(Lifecycle.State.STARTED) {
                graph.settings.values.collect { values ->
                    if (values.keepScreenOn) {
                        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
                    } else {
                        window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
                    }
                }
            }
        }
        val tab = intent.getStringExtra("tab")
        val benchmark = intent.getIntExtra("benchmark", 0).takeIf { it > 0 }
        setContent {
            JetlinkTheme {
                RootScreen(graph = graph, initialTab = tab, launchBenchmark = benchmark)
            }
        }
    }
}
