BUILD_DIR = File.join(__dir__, 'build', 'debug')
WEB_BUILD_DIR = File.join(__dir__, 'build', 'web')
PWD = File.expand_path(__dir__)
PID_FILE = File.join(BUILD_DIR, 'raptor.pid')
EXE_FILE = File.join(BUILD_DIR, 'Raptor')

directory BUILD_DIR do
  sh "cmake -S #{PWD} -B #{BUILD_DIR} -G Ninja -DCMAKE_BUILD_TYPE=Debug"
end

directory WEB_BUILD_DIR do
  sh "emcmake cmake -S #{PWD} -B #{WEB_BUILD_DIR} -G Ninja -DCMAKE_BUILD_TYPE=Release"
end

desc 'Compile the project'
task compile: BUILD_DIR do
  sh "cmake --build #{BUILD_DIR} --parallel"
end

def run(cmd)
  puts "Executing: #{cmd}"
  pid = Process.spawn(cmd, chdir: BUILD_DIR)
  File.write(PID_FILE, pid)
  puts "Started process #{pid} with PID file at #{PID_FILE}"
ensure
  Process.wait(pid)
  File.delete(PID_FILE) if File.exist?(PID_FILE)
end

desc 'Run the project'
task run: :compile do
  run EXE_FILE
end

desc 'Compile for web'
task web: WEB_BUILD_DIR do
  sh "cmake --build #{WEB_BUILD_DIR} --parallel"
end

SHELL_STATIC_FILES = [
  'shell/logo.png',
  'shell/shell.css',
].freeze

namespace :web do
  desc 'Package the web build into a zip file'
  task pack: :web do
    require 'zip'
    require 'zlib'
    require 'stringio'

    basename = File.join(WEB_BUILD_DIR, 'Raptor')
    project_name = 'Raptor'
    output_file = File.join(WEB_BUILD_DIR, "#{project_name}.zip")

    File.delete(output_file) if File.exist?(output_file)

    Zip::File.open(output_file, create: true) do |zipfile|
      # Pack .js file
      js_file = "#{basename}.js"
      puts "Packing #{js_file}"
      zipfile.add("#{project_name}.js", js_file)

      # Pack .wasm file (gzip compressed as .wasmz, stored without additional zip compression)
      wasm_file = "#{basename}.wasm"
      puts "Packing #{wasm_file}"
      wasm_data = File.binread(wasm_file)

      # Gzip compress the wasm data
      sio = StringIO.new
      Zlib::GzipWriter.wrap(sio) { |gz| gz.write(wasm_data) }
      gzipped_wasm = sio.string

      # Store in zip without additional compression (like Python's ZIP_STORED)
      zipfile.get_output_stream("#{project_name}.wasmz") do |stream|
        stream.write(gzipped_wasm)
      end

      # Pack .data file if it exists
      data_file = "#{basename}.data"
      if File.exist?(data_file)
        puts "Packing #{data_file}"
        zipfile.add("#{project_name}.data.pck", data_file)
      end

      # Pack debug wasm if it exists
      debug_wasm = "#{basename}.wasm.debug.wasm"
      if File.exist?(debug_wasm)
        puts "Packing #{debug_wasm}"
        zipfile.add("#{project_name}.wasm.debug.wasm", debug_wasm)
      end

      # Pack shell static files
      SHELL_STATIC_FILES.each do |file|
        file_path = File.join(PWD, file)
        puts "Packing #{file_path}"
        zipfile.add(file, file_path)
      end

      # Pack templated index.html
      template_file = File.join(PWD, 'shell/shell.html')
      puts "Packing #{template_file}"
      template = File.read(template_file)
      template.gsub!('{{SCRIPT_NAME}}', "#{project_name}.js")
      zipfile.get_output_stream('index.html') { |s| s.write(template) }

      # Pack templated shell.js
      shell_template = File.join(PWD, 'shell/shell.js')
      puts "Packing #{shell_template}"
      shell_js = File.read(shell_template)
      shell_js.gsub!('{{UNPACKED_WASM_SIZE}}', wasm_data.bytesize.to_s)
      zipfile.get_output_stream('shell.js') { |s| s.write(shell_js) }
    end

    puts "Created #{output_file}"
  end

  desc "Publish to itch.io"
  task :publish do
    sh "butler push build/web/Raptor.zip pusewicz/raptor-cute-c:html5"
  end
end

namespace :run do
  desc 'Run the project with debugger attached'
  task :debugger do
    run "lldb #{EXE_FILE} -o run"
  end
end

desc 'Watch for changes and recompile'
task :watch do
  require 'listen'
  listener = Listen.to('src', 'include') do |_modified, _added, _removed|
    if File.exist?(PID_FILE)
      pid = File.read(PID_FILE).to_i

      begin
        Process.kill(0, pid)
        Rake::Task[:compile].execute
        puts "Sending SIGHUP to process #{pid}"
        Process.kill('SIGHUP', pid)
      rescue Errno::ESRCH
        warn "Process #{pid} not found"
      end
    else
      warn "No PID file found at #{PID_FILE}, not sending SIGHUP"
    end
  end
  puts 'Listening for changes...'
  listener.start
  sleep
end

namespace :headers do
  desc 'Update header files'
  task :update do
    {
      'include/pico_ecs.h' => 'https://raw.githubusercontent.com/empyreanx/pico_headers/refs/heads/main/pico_ecs.h'
    }.each do |local_path, url|
      sh "curl -o #{local_path} #{url}"
    end
  end
end

desc "Formats the source code using clang-format"
task :format do
  sh %(fd --full-path src/ -e "c" -e "h" | xargs clang-format -i)
end

task default: :compile
