import JetlinkKit
import JetlinkUI
import SwiftUI

/// Everything known about one model, shown in the Models inspector, with the
/// actions that apply to it right under its facts.
struct ModelDetailView<Actions: View>: View {
  let row: ModelRow
  @ViewBuilder let actions: Actions

  var body: some View {
    Form {
      Section {
        LabeledContent("Name", value: row.name)
        if let ref = row.ref {
          longValue("Ref", ref)
        }
        if let sha = row.sha256 {
          longValue("SHA-256", sha)
        }
        LabeledContent("Size") { ByteCount(row.bytes) }
        if !BuildTime.text(row.buildTime).isEmpty {
          LabeledContent("Built", value: BuildTime.text(row.buildTime))
        }
        if let checkpoint = currentArtifact?.checkpoint {
          longValue("Checkpoint", checkpoint)
        }
        LabeledContent("Status") {
          ModelStatusLabel(row.status)
        }
      }

      actions

      Section("Prepared Engines") {
        if row.preparedFor.isEmpty {
          Text("No prepared engine yet.")
            .foregroundStyle(.secondary)
        } else {
          ForEach(row.preparedFor) { artifact in
            VStack(alignment: .leading, spacing: 2) {
              Text(ModelFormatting.backendName(artifact.backend))
              Text(artifact.device)
                .font(.callout)
                .foregroundStyle(.secondary)
              Text(detailLine(artifact))
                .font(.callout)
                .foregroundStyle(.secondary)
              Text(artifact.path)
                .font(.system(.callout, design: .monospaced))
                .foregroundStyle(.secondary)
                .textSelection(.enabled)
                .fixedSize(horizontal: false, vertical: true)
                .frame(maxWidth: .infinity, alignment: .leading)
            }
            .padding(.vertical, 2)
          }
        }
      }
    }
    .formStyle(.grouped)
    .frame(minWidth: 280)
  }

  /// A value too long for a label and a value on one line: hex, paths, ids.
  private func longValue(_ label: String, _ value: String) -> some View {
    VStack(alignment: .leading, spacing: 2) {
      Text(label)
        .foregroundStyle(.secondary)
      Text(value)
        .font(.system(.callout, design: .monospaced))
        .textSelection(.enabled)
        .fixedSize(horizontal: false, vertical: true)
        .frame(maxWidth: .infinity, alignment: .leading)
    }
  }

  private var currentArtifact: InventoryArtifact? {
    row.preparedFor.first { $0.current } ?? row.preparedFor.first
  }

  private func detailLine(_ artifact: InventoryArtifact) -> String {
    var parts: [String] = []
    if let version = artifact.runtimeVersion, !version.isEmpty {
      parts.append(version)
    }
    let built = BuildTime.text(artifact.builtAt)
    if !built.isEmpty {
      parts.append("built \(built)")
    }
    if let seconds = artifact.buildSeconds, seconds > 0 {
      parts.append("took \(Duration.seconds(Int(seconds.rounded())).formatted(.units(allowed: [.hours, .minutes, .seconds], width: .abbreviated)))")
    }
    parts.append(ByteCount.string(artifact.bytes))
    return parts.joined(separator: ", ")
  }
}

#Preview {
  ModelDetailView(row: PreviewData.loadedRow) {
    Section("Actions") {
      Button("Stop Using Model") {}
      Button("Show in Finder") {}
    }
  }
  .environment(ModelStore.preview(catalog: PreviewData.catalog, inventory: PreviewData.inventory, engine: PreviewData.engineReady))
  .frame(width: 320, height: 560)
}
