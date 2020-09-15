package io.flutter.plugins;

import androidx.annotation.VisibleForTesting;
import io.flutter.embedding.engine.FlutterEngine;
import java.util.ArrayList;
import java.util.List;

/**
 * A fake of the {@code GeneratedPluginRegistrant} normally built by the tool into Flutter apps.
 *
 * <p>Used to test engine logic which interacts with the generated class.
 */
@VisibleForTesting
public class GeneratedPluginRegistrant {
  private static final List<FlutterEngine> registeredEngines = new ArrayList<>();

  /**
   * The one and only method currently generated by the tool.
   *
   * <p>Normally it registers all plugins in an app with the given {@code engine}. This fake tracks
   * all registered engines instead.
   */
  public static void registerWith(FlutterEngine engine) {
    registeredEngines.add(engine);
  }

  /**
   * Clears the mutable static state regrettably stored in this class.
   *
   * <p>{@link #registerWith} is a static call with no visible side effects. In order to verify when
   * it's been called we also unfortunately need to store the state statically. This should be
   * called before and after each test run accessing this class to make sure the state is clear both
   * before and after the run.
   */
  public static void clearRegisteredEngines() {
    registeredEngines.clear();
  }

  /**
   * Returns a list of all the engines registered so far.
   *
   * <p>CAUTION: This list is static and must be manually wiped in between test runs. See {@link
   * #clearRegisteredEngines()}.
   */
  public static List<FlutterEngine> getRegisteredEngines() {
    return new ArrayList<>(registeredEngines);
  }
}
